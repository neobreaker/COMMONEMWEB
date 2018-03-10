#include "httpServer.h"
#include "httpParser.h"
#include "httpUtil.h"
#include <string.h>
#include <stdio.h>

#ifndef DATA_BUF_SIZE
#define DATA_BUF_SIZE       2048
#endif

static st_http_request* http_request;               /**< Pointer to received HTTP request */
static st_http_request* parsed_http_request;        /**< Pointer to parsed HTTP request */
static uint8_t* http_response;                      /**< Pointer to HTTP response */

// Number of registered web content in code flash memory
static uint16_t total_content_cnt = 0;

uint8_t* pHTTP_TX;
uint8_t* pHTTP_RX;

st_http_socket HTTPSock_Status[SOCK_NUM] = { {STATE_HTTP_IDLE, }, };
httpServer_webContent web_content[MAX_CONTENT_CALLBACK];

static void http_process_handler ( int s, socket_cfg_t* cfg, st_http_request* p_http_request );
static void send_http_response_header ( int s, socket_cfg_t* cfg, uint8_t content_type, uint32_t body_len, uint16_t http_status );
static void send_http_response_body ( int s, socket_cfg_t* cfg, uint8_t* uri_name, uint8_t* buf, uint32_t start_addr, uint32_t file_len );
static void send_http_response_cgi(uint8_t s, socket_cfg_t* cfg, uint8_t * buf, uint8_t * http_body, uint16_t file_len);

void httpServer_init ( uint8_t* tx_buf, uint8_t* rx_buf )
{
	// User's shared buffer
	pHTTP_TX = tx_buf;
	pHTTP_RX = rx_buf;
}

void httpServer_run ( int* s, socket_cfg_t* cfg )
{
	int err;
	uint16_t len;
	uint32_t gettime = 0;
	socklen_t addr_len;
	struct sockaddr_in conn_addr;
	struct sockaddr_in server_addr;
	http_request = ( st_http_request* ) pHTTP_RX;   // Structure of HTTP Request
	parsed_http_request = ( st_http_request* ) pHTTP_TX;

	/* HTTP Service Start */
	switch ( cfg->socket_status ( *s ) )
	{
		case SOCK_ESTABLISHED:

			// HTTP Process states
			switch ( HTTPSock_Status[*s].sock_status )
			{

				case STATE_HTTP_IDLE :
					len = cfg->recvfrom ( *s, ( unsigned int* ) http_request, 1024, 0, &conn_addr, &addr_len );
					if ( len == 0 )
					{
						return;
					}
					else if ( len > DATA_BUF_SIZE )
					{
						len = DATA_BUF_SIZE;
					}

					* ( ( ( uint8_t* ) http_request ) + len ) = '\0';

					parse_http_request ( parsed_http_request, ( uint8_t* ) http_request );

					// HTTP 'response' handler; includes send_http_response_header / body function
					http_process_handler ( *s, cfg, parsed_http_request );

					if ( HTTPSock_Status[*s].file_len > 0 )
					{
						HTTPSock_Status[*s].sock_status = STATE_HTTP_RES_INPROC;
					}
					else
					{
						HTTPSock_Status[*s].sock_status = STATE_HTTP_RES_DONE;    // Send the 'HTTP response' end
					}

					break;

				case STATE_HTTP_RES_INPROC :
					// Repeat: Send the remain parts of HTTP responses

					// Repeatedly send remaining data to client
					send_http_response_body ( *s, cfg, 0, http_response, 0, 0 );

					if ( HTTPSock_Status[*s].file_len == 0 )
					{
						HTTPSock_Status[*s].sock_status = STATE_HTTP_RES_DONE;
					}
					break;

				case STATE_HTTP_RES_DONE :

					// Socket file info structure re-initialize
					HTTPSock_Status[*s].file_len = 0;
					HTTPSock_Status[*s].file_offset = 0;
					HTTPSock_Status[*s].file_start = 0;
					HTTPSock_Status[*s].sock_status = STATE_HTTP_IDLE;


					cfg->disconnect ( *s );
					break;

				default :
					break;
			}

			break;

		case SOCK_CLOSE_WAIT:

			cfg->close ( *s );
			break;

		case SOCK_CLOSED:
			//socket(s, Sn_MR_TCP, HTTP_SERVER_PORT, 0x00) == s);    /* Reinitialize the socket */
			*s = cfg->socket ( AF_INET, SOCK_STREAM, 0 );
			if ( *s == -1 )
			{
				return ;
			}
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr =htonl ( INADDR_ANY );
			server_addr.sin_port = 80;

			err = cfg->bind ( *s,  &server_addr, sizeof ( server_addr ) );
			if ( err < 0 )
			{
				return ;
			}

			break;

		case SOCK_INIT:
			err = cfg->listen ( *s, 1 );
			if ( err < 0 )
			{
				return ;
			}
			break;

		case SOCK_LISTEN:
			break;

		default :
			break;

	} // end of switch

}

static void http_process_handler ( int s, socket_cfg_t* cfg, st_http_request* p_http_request )
{
	uint8_t* uri_name;
	uint32_t content_addr = 0;
	uint16_t content_num = 0;
	uint32_t file_len = 0;

	uint8_t uri_buf[MAX_URI_SIZE]= {0x00, };

	uint16_t http_status;
	uint8_t content_found;

	http_status = 0;
	http_response = pHTTP_RX;
	file_len = 0;

	//method Analyze
	switch ( p_http_request->METHOD )
	{
		case METHOD_ERR :
			http_status = STATUS_BAD_REQ;
			send_http_response_header ( s, cfg, 0, 0, http_status );
			break;

		case METHOD_HEAD :
		case METHOD_GET :
			get_http_uri_name ( p_http_request->URI, uri_buf );
			uri_name = uri_buf;

			if ( !strcmp ( ( char* ) uri_name, "/" ) )
			{
				strcpy ( ( char* ) uri_name, INITIAL_WEBPAGE );    // If URI is "/", respond by index.html
			}
			else if ( !strcmp ( ( char* ) uri_name, "m" ) )
			{
				strcpy ( ( char* ) uri_name, M_INITIAL_WEBPAGE );
			}
			else if ( !strcmp ( ( char* ) uri_name, "mobile" ) )
			{
				strcpy ( ( char* ) uri_name, MOBILE_INITIAL_WEBPAGE );
			}

			find_http_uri_type ( &p_http_request->TYPE, uri_name ); // Checking requested file types (HTML, TEXT, GIF, JPEG and Etc. are included)

			if ( p_http_request->TYPE == PTYPE_CGI )
			{
				content_found = http_get_cgi_handler ( uri_name, pHTTP_TX, &file_len );
				if ( content_found && ( file_len <= ( DATA_BUF_SIZE- ( strlen ( RES_CGIHEAD_OK )+8 ) ) ) )
				{
					//send_http_response_cgi(s, http_response, pHTTP_TX, (uint16_t)file_len);
				}
				else
				{
					send_http_response_header ( s, cfg, PTYPE_CGI, 0, STATUS_NOT_FOUND );
				}
			}
			else
			{
				// Find the User registered index for web content
				if ( find_userReg_webContent ( uri_buf, &content_num, &file_len ) )
				{
					content_found = 1; // Web content found in code flash memory
					content_addr = ( uint32_t ) content_num;
					HTTPSock_Status[s].storage_type = CODEFLASH;
				}
				// Not CGI request, Web content in 'SD card' or 'Data flash' requested
				else
				{
					content_found = 0; // fail to find content
				}

				if ( !content_found )
				{
					http_status = STATUS_NOT_FOUND;
				}
				else
				{
					http_status = STATUS_OK;
				}

				// Send HTTP header
				if ( http_status )
				{
					send_http_response_header ( s, cfg, p_http_request->TYPE, file_len, http_status );
				}

				// Send HTTP body (content)
				if ( http_status == STATUS_OK )
				{
					send_http_response_body ( s, cfg, uri_name, http_response, content_addr, file_len );
				}
			}

			break;

		case METHOD_POST :

			mid ( ( char* ) p_http_request->URI, "/", " HTTP", ( char* ) uri_buf );
			uri_name = uri_buf;
			find_http_uri_type ( &p_http_request->TYPE, uri_name ); // Check file type (HTML, TEXT, GIF, JPEG are included)

			if ( p_http_request->TYPE == PTYPE_CGI ) // HTTP POST Method; CGI Process
			{
				content_found = http_post_cgi_handler ( uri_name, p_http_request, http_response, &file_len );

				if ( content_found && ( file_len <= ( DATA_BUF_SIZE- ( strlen ( RES_CGIHEAD_OK )+8 ) ) ) )
				{
					send_http_response_cgi ( s, cfg, pHTTP_TX, http_response, ( uint16_t ) file_len );

					// Reset the H/W for apply to the change configuration information
					if ( content_found == HTTP_RESET )
					{
						//HTTPServer_ReStart();
					}
				}
				else
				{
					send_http_response_header ( s, cfg, PTYPE_CGI, 0, STATUS_NOT_FOUND );
				}
			}
			else    // HTTP POST Method; Content not found
			{
				send_http_response_header ( s, cfg, 0, 0, STATUS_NOT_FOUND );
			}
			break;

		default :
			http_status = STATUS_BAD_REQ;
			send_http_response_header ( s, cfg, 0, 0, http_status );
			break;
	}
}

////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////
static void send_http_response_header ( int s, socket_cfg_t* cfg, uint8_t content_type, uint32_t body_len, uint16_t http_status )
{
	switch ( http_status )
	{
		case STATUS_OK:         // HTTP/1.1 200 OK
			if ( ( content_type != PTYPE_CGI ) && ( content_type != PTYPE_XML ) )   // CGI/XML type request does not respond HTTP header
			{
				make_http_response_head ( ( char* ) http_response, content_type, body_len );
			}
			else
			{
				// CGI/XML type request does not respond HTTP header to client
				http_status = 0;
			}
			break;
		case STATUS_BAD_REQ:    // HTTP/1.1 400 OK
			memcpy ( http_response, ERROR_REQUEST_PAGE, sizeof ( ERROR_REQUEST_PAGE ) );
			break;
		case STATUS_NOT_FOUND:  // HTTP/1.1 404 Not Found
			memcpy ( http_response, ERROR_HTML_PAGE, sizeof ( ERROR_HTML_PAGE ) );
			break;
		default:
			break;
	}

	// Send the HTTP Response 'header'
	if ( http_status )
	{
		cfg->send ( s, ( unsigned int* ) http_response, strlen ( ( char* ) http_response ), 0 );
	}
}


uint8_t find_userReg_webContent ( uint8_t* content_name, uint16_t* content_num, uint32_t* file_len )
{
	uint16_t i;
	uint8_t ret = 0; // '0' means 'File Not Found'

	for ( i = 0; i < total_content_cnt; i++ )
	{
		if ( !strcmp ( ( char* ) content_name, ( char* ) web_content[i].content_name ) )
		{
			*file_len = web_content[i].content_len;
			*content_num = i;
			ret = 1; // If the requested content found, ret set to '1' (Found)
			break;
		}
	}
	return ret;
}

uint16_t read_userReg_webContent ( uint16_t content_num, uint8_t* buf, uint32_t offset, uint16_t size )
{
	uint16_t ret = 0;
	uint8_t* ptr;

	if ( content_num > total_content_cnt )
	{
		return 0;
	}

	ptr = web_content[content_num].content;
	if ( offset )
	{
		ptr += offset;
	}

	strncpy ( ( char* ) buf, ( char* ) ptr, size );
	* ( buf+size ) = 0; // Insert '/0' for indicates the 'End of String' (null terminated)

	ret = strlen ( ( void* ) buf );
	return ret;
}

void reg_httpServer_webContent ( uint8_t* content_name, uint8_t* content,socket_cfg_t* cfg )
{
	uint16_t name_len;
	uint32_t content_len;

	if ( content_name == NULL || content == NULL )
	{
		return;
	}
	else if ( total_content_cnt >= MAX_CONTENT_CALLBACK )
	{
		return;
	}

	name_len = strlen ( ( char* ) content_name );
	content_len = strlen ( ( char* ) content );

	web_content[total_content_cnt].content_name = cfg->malloc ( name_len+1 );
	strcpy ( ( char* ) web_content[total_content_cnt].content_name, ( const char* ) content_name );
	web_content[total_content_cnt].content_len = content_len;
	web_content[total_content_cnt].content = content;

	total_content_cnt++;
}

static void send_http_response_body ( int s, socket_cfg_t* cfg, uint8_t* uri_name, uint8_t* buf, uint32_t start_addr, uint32_t file_len )
{
	uint32_t send_len;

	uint8_t flag_datasend_end = 0;


	if ( s == -1 )
	{
		return;    // exception handling; invalid number
	}

	// Send the HTTP Response 'body'; requested file
	if ( !HTTPSock_Status[s].file_len )   // ### Send HTTP response body: First part ###
	{
		if ( file_len > DATA_BUF_SIZE - 1 )
		{
			HTTPSock_Status[s].file_start = start_addr;
			HTTPSock_Status[s].file_len = file_len;
			send_len = DATA_BUF_SIZE - 1;

			memset ( HTTPSock_Status[s].file_name, 0x00, MAX_CONTENT_NAME_LEN );
			strcpy ( ( char* ) HTTPSock_Status[s].file_name, ( char* ) uri_name );

		}
		else
		{
			// Send process end
			send_len = file_len;
		}
	}
	else   // remained parts
	{
		send_len = HTTPSock_Status[s].file_len - HTTPSock_Status[s].file_offset;

		if ( send_len > DATA_BUF_SIZE - 1 )
		{
			send_len = DATA_BUF_SIZE - 1;
			//HTTPSock_Status[get_seqnum]->file_offset += send_len;
		}
		else
		{
			// Send process end
			flag_datasend_end = 1;
		}
	}

	/*****************************************************/
	//HTTPSock_Status[get_seqnum]->storage_type == NONE
	//HTTPSock_Status[get_seqnum]->storage_type == CODEFLASH
	//HTTPSock_Status[get_seqnum]->storage_type == SDCARD
	//HTTPSock_Status[get_seqnum]->storage_type == DATAFLASH
	/*****************************************************/

	if ( HTTPSock_Status[s].storage_type == CODEFLASH )
	{
		if ( HTTPSock_Status[s].file_len )
		{
			start_addr = HTTPSock_Status[s].file_start;
		}
		read_userReg_webContent ( start_addr, &buf[0], HTTPSock_Status[s].file_offset, send_len );
	}
	else
	{
		send_len = 0;
	}
	// Requested content send to HTTP client

	if ( send_len )
	{
		cfg->send ( s, ( unsigned int* ) buf, send_len, 0 );
	}
	else
	{
		flag_datasend_end = 1;
	}

	if ( flag_datasend_end )
	{
		HTTPSock_Status[s].file_start = 0;
		HTTPSock_Status[s].file_len = 0;
		HTTPSock_Status[s].file_offset = 0;
		flag_datasend_end = 0;
	}
	else
	{
		HTTPSock_Status[s].file_offset += send_len;
	}

}

static void send_http_response_cgi(uint8_t s, socket_cfg_t* cfg, uint8_t * buf, uint8_t * http_body, uint16_t file_len)
{
	uint16_t send_len = 0;

	send_len = sprintf((char *)buf, "%s%d\r\n\r\n%s", RES_CGIHEAD_OK, file_len, http_body);

	cfg->send(s, buf, send_len, 0);
}

