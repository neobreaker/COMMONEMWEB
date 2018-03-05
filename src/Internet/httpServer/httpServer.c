#include "httpServer.h"
#include "httpParser.h"
#include "httpUtil.h"
#include <string.h>

#ifndef DATA_BUF_SIZE
#define DATA_BUF_SIZE       2048
#endif

static st_http_request * http_request;              /**< Pointer to received HTTP request */
static st_http_request * parsed_http_request;       /**< Pointer to parsed HTTP request */
static uint8_t * http_response;                     /**< Pointer to HTTP response */

// Number of registered web content in code flash memory
static uint16_t total_content_cnt = 0;

uint8_t * pHTTP_TX;
uint8_t * pHTTP_RX;

st_http_socket HTTPSock_Status[SOCK_NUM] = { {STATE_HTTP_IDLE, }, };
httpServer_webContent web_content[MAX_CONTENT_CALLBACK];

static void http_process_handler(int s, st_http_request * p_http_request);
static void send_http_response_header(int s, uint8_t content_type, uint32_t body_len, uint16_t http_status);

void httpServer_init(uint8_t * tx_buf, uint8_t * rx_buf)
{
    // User's shared buffer
    pHTTP_TX = tx_buf;
    pHTTP_RX = rx_buf;
}

void httpServer_run(int* s, socket_port_t *port)
{
    int err;
    uint16_t len;
    uint32_t gettime = 0;
    socklen_t addr_len;
    struct sockaddr_in conn_addr;

    http_request = (st_http_request *)pHTTP_RX;     // Structure of HTTP Request
    parsed_http_request = (st_http_request *)pHTTP_TX;

    /* HTTP Service Start */
    switch(socket_status(*s))
    {
        case SOCK_ESTABLISHED:

            // HTTP Process states
            switch(HTTPSock_Status[*s].sock_status)
            {

                case STATE_HTTP_IDLE :

                    if (len > DATA_BUF_SIZE) len = DATA_BUF_SIZE;
                    len = recvfrom(*s, (unsigned int *)http_request, 1024, 0, &conn_addr, &addr_len);

                    *(((uint8_t *)http_request) + len) = '\0';
                    
                    parse_http_request(parsed_http_request, (uint8_t *)http_request);
					
                    // HTTP 'response' handler; includes send_http_response_header / body function
                    http_process_handler(*s, parsed_http_request);
					/*
                    gettime = get_httpServer_timecount();
                    // Check the TX socket buffer for End of HTTP response sends
                    while(getSn_TX_FSR(s) != (getSn_TXBUF_SIZE(s)*1024))
                    {
                        if((get_httpServer_timecount() - gettime) > 3)
                        {

                            break;
                        }
                    }

                    if(HTTPSock_Status[seqnum].file_len > 0) HTTPSock_Status[seqnum].sock_status = STATE_HTTP_RES_INPROC;
                    else HTTPSock_Status[seqnum].sock_status = STATE_HTTP_RES_DONE; // Send the 'HTTP response' end
                    */

                    break;

                case STATE_HTTP_RES_INPROC :
                    // Repeat: Send the remain parts of HTTP responses

                    // Repeatedly send remaining data to client
                    //send_http_response_body(s, 0, http_response, 0, 0);

                    //if(HTTPSock_Status[seqnum].file_len == 0) HTTPSock_Status[seqnum].sock_status = STATE_HTTP_RES_DONE;
                    break;

                case STATE_HTTP_RES_DONE :
                    /*
                    // Socket file info structure re-initialize
                    HTTPSock_Status[seqnum].file_len = 0;
                    HTTPSock_Status[seqnum].file_offset = 0;
                    HTTPSock_Status[seqnum].file_start = 0;
                    HTTPSock_Status[seqnum].sock_status = STATE_HTTP_IDLE;


                    http_disconnect(s);*/
                    break;

                default :
                    break;
            }

            break;

        case SOCK_CLOSE_WAIT:

            //disconnect(s);
            break;

        case SOCK_CLOSED:
            break;

        case SOCK_INIT:
            err = listen(*s, 1);
            if (err < 0)
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

static void http_process_handler(int s, st_http_request * p_http_request)
{
    uint8_t * uri_name;
    uint32_t content_addr = 0;
    uint16_t content_num = 0;
    uint32_t file_len = 0;

    uint8_t uri_buf[MAX_URI_SIZE]= {0x00, };

    uint16_t http_status;
    int8_t get_seqnum;
    uint8_t content_found;

    http_status = 0;
    http_response = pHTTP_RX;
    file_len = 0;

    //method Analyze
    switch (p_http_request->METHOD)
    {
        case METHOD_ERR :
            http_status = STATUS_BAD_REQ;
            send_http_response_header(s, 0, 0, http_status);
            break;

        case METHOD_HEAD :
        case METHOD_GET :
            get_http_uri_name(p_http_request->URI, uri_buf);
            uri_name = uri_buf;

            if (!strcmp((char *)uri_name, "/")) strcpy((char *)uri_name, INITIAL_WEBPAGE);  // If URI is "/", respond by index.html
            if (!strcmp((char *)uri_name, "m")) strcpy((char *)uri_name, M_INITIAL_WEBPAGE);
            if (!strcmp((char *)uri_name, "mobile")) strcpy((char *)uri_name, MOBILE_INITIAL_WEBPAGE);
            find_http_uri_type(&p_http_request->TYPE, uri_name);    // Checking requested file types (HTML, TEXT, GIF, JPEG and Etc. are included)

            if(p_http_request->TYPE == PTYPE_CGI)
            {
                content_found = http_get_cgi_handler(uri_name, pHTTP_TX, &file_len);
                if(content_found && (file_len <= (DATA_BUF_SIZE-(strlen(RES_CGIHEAD_OK)+8))))
                {
                    //send_http_response_cgi(s, http_response, pHTTP_TX, (uint16_t)file_len);
                }
                else
                {
                    send_http_response_header(s, PTYPE_CGI, 0, STATUS_NOT_FOUND);
                }
            }
            else
            {
                // Find the User registered index for web content
                if(find_userReg_webContent(uri_buf, &content_num, &file_len))
                {
                    content_found = 1; // Web content found in code flash memory
                    content_addr = (uint32_t)content_num;
                    HTTPSock_Status[get_seqnum].storage_type = CODEFLASH;
                }
                // Not CGI request, Web content in 'SD card' or 'Data flash' requested
                else
                {
                    content_found = 0; // fail to find content
                }

                if(!content_found)
                {
                    http_status = STATUS_NOT_FOUND;
                }
                else
                {
                    http_status = STATUS_OK;
                }

                // Send HTTP header
                if(http_status)
                {
                    send_http_response_header(s, p_http_request->TYPE, file_len, http_status);
                }

                // Send HTTP body (content)
                if(http_status == STATUS_OK)
                {
                    //send_http_response_body(s, uri_name, http_response, content_addr, file_len);
                }
            }
            
            break;

        case METHOD_POST :
			/*
            mid((char *)p_http_request->URI, "/", " HTTP", (char *)uri_buf);
            uri_name = uri_buf;
            find_http_uri_type(&p_http_request->TYPE, uri_name);    // Check file type (HTML, TEXT, GIF, JPEG are included)

#ifdef _HTTPSERVER_DEBUG_
            printf("\r\n> HTTPSocket[%d] : HTTP Method POST\r\n", s);
            printf("> HTTPSocket[%d] : Request URI = %s ", s, uri_name);
            printf("Type = %d\r\n", p_http_request->TYPE);
#endif

            if(p_http_request->TYPE == PTYPE_CGI)   // HTTP POST Method; CGI Process
            {
                content_found = http_post_cgi_handler(uri_name, p_http_request, http_response, &file_len);
#ifdef _HTTPSERVER_DEBUG_
                printf("> HTTPSocket[%d] : [CGI: %s] / Response len [ %ld ]byte\r\n", s, content_found?"Content found":"Content not found", file_len);
#endif
                if(content_found && (file_len <= (DATA_BUF_SIZE-(strlen(RES_CGIHEAD_OK)+8))))
                {
                    send_http_response_cgi(s, pHTTP_TX, http_response, (uint16_t)file_len);

                    // Reset the H/W for apply to the change configuration information
                    if(content_found == HTTP_RESET) HTTPServer_ReStart();
                }
                else
                {
                    send_http_response_header(s, PTYPE_CGI, 0, STATUS_NOT_FOUND);
                }
            }
            else    // HTTP POST Method; Content not found
            {
                send_http_response_header(s, 0, 0, STATUS_NOT_FOUND);
            }*/
            break;

        default :
            http_status = STATUS_BAD_REQ;
            send_http_response_header(s, 0, 0, http_status);
            break;
    }
}

////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////
static void send_http_response_header(int s, uint8_t content_type, uint32_t body_len, uint16_t http_status)
{
    switch(http_status)
    {
        case STATUS_OK:         // HTTP/1.1 200 OK
            if((content_type != PTYPE_CGI) && (content_type != PTYPE_XML)) // CGI/XML type request does not respond HTTP header
            {
                make_http_response_head((char*)http_response, content_type, body_len);
            }
            else
            {
                // CGI/XML type request does not respond HTTP header to client
                http_status = 0;
            }
            break;
        case STATUS_BAD_REQ:    // HTTP/1.1 400 OK
            memcpy(http_response, ERROR_REQUEST_PAGE, sizeof(ERROR_REQUEST_PAGE));
            break;
        case STATUS_NOT_FOUND:  // HTTP/1.1 404 Not Found
            memcpy(http_response, ERROR_HTML_PAGE, sizeof(ERROR_HTML_PAGE));
            break;
        default:
            break;
    }

    // Send the HTTP Response 'header'
    if(http_status)
    {
        send(s, (unsigned int *)http_response, strlen((char *)http_response), 0);
    }
}

uint8_t find_userReg_webContent(uint8_t * content_name, uint16_t * content_num, uint32_t * file_len)
{
    uint16_t i;
    uint8_t ret = 0; // '0' means 'File Not Found'

    for(i = 0; i < total_content_cnt; i++)
    {
        if(!strcmp((char *)content_name, (char *)web_content[i].content_name))
        {
            *file_len = web_content[i].content_len;
            *content_num = i;
            ret = 1; // If the requested content found, ret set to '1' (Found)
            break;
        }
    }
    return ret;
}

