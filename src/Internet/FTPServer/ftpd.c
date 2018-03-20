/*
* Wiznet.
* (c) Copyright 2002, Wiznet.
*
* Filename	: ftpd.c
* Version	: 1.0
* Programmer(s)	:
* Created	: 2003/01/28
* Description   : FTP daemon. (AVR-GCC Compiler)
*/


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include "stdio_private.h"
#include "w5500_port.h"
#include "ftpd.h"
#include "httpParser.h"

/* Command table */
static char* commands[] =
{
	"user",
	"acct",
	"pass",
	"type",
	"list",
	"cwd",
	"dele",
	"name",
	"quit",
	"retr",
	"stor",
	"port",
	"nlst",
	"pwd",
	"xpwd",
	"mkd",
	"xmkd",
	"xrmd",
	"rmd ",
	"stru",
	"mode",
	"syst",
	"xmd5",
	"xcwd",
	"feat",
	"pasv",
	"size",
	"mlsd",
	"appe",
	NULL
};


static int s_ctrl_socket = -1;
static int s_data_socket = -1;

#if 0
/* Response messages */
char banner[] = "220 %s FTP version %s ready.\r\n";
char badcmd[] = "500 Unknown command '%s'\r\n";
char binwarn[] = "100 Warning: type is ASCII and %s appears to be binary\r\n";
char unsupp[] = "500 Unsupported command or option\r\n";
char givepass[] = "331 Enter PASS command\r\n";
char logged[] = "230 Logged in\r\n";
char typeok[] = "200 Type %s OK\r\n";
char only8[] = "501 Only logical bytesize 8 supported\r\n";
char deleok[] = "250 File deleted\r\n";
char mkdok[] = "200 MKD ok\r\n";
char delefail[] = "550 Delete failed: %s\r\n";
char pwdmsg[] = "257 \"%s\" is current directory\r\n";
char badtype[] = "501 Unknown type \"%s\"\r\n";
char badport[] = "501 Bad port syntax\r\n";
char unimp[] = "502 Command does not implemented yet.\r\n";
char bye[] = "221 Goodbye!\r\n";
char nodir[] = "553 Can't read directory \"%s\": %s\r\n";
char cantopen[] = "550 Can't read file \"%s\": %s\r\n";
char sending[] = "150 Opening data connection for %s (%d.%d.%d.%d,%d)\r\n";
char cantmake[] = "553 Can't create \"%s\": %s\r\n";
char writerr[] = "552 Write error: %s\r\n";
char portok[] = "200 PORT command successful.\r\n";
char rxok[] = "226 Transfer complete.\r\n";
char txok[] = "226 Transfer complete.\r\n";
char noperm[] = "550 Permission denied\r\n";
char noconn[] = "425 Data connection reset\r\n";
char lowmem[] = "421 System overloaded, try again later\r\n";
char notlog[] = "530 Please log in with USER and PASS\r\n";
char userfirst[] = "503 Login with USER first.\r\n";
char okay[] = "200 Ok\r\n";
char syst[] = "215 %s Type: L%d Version: %s\r\n";
char sizefail[] = "550 File not found\r\n";
#endif

static un_l2cval s_remote_ip;
static uint16_t  s_remote_port;
static un_l2cval s_local_ip;
static uint16_t  s_local_port;
static uint8_t s_connect_state_control = 0;
static uint8_t s_connect_state_data = 0;

static struct ftpd ftp;

/*static int current_year = 2018;
static int current_month = 3;
static int current_day = 19;
static int current_hour = 11;
static int current_min = 12;
static int current_sec = 50;
*/

int fsprintf ( uint8_t s, const char* format, ... )
{
	int i;
	/*
		char buf[LINELEN];
		FILE f;
		va_list ap;

		f.flags = __SWR | __SSTR;
		f.buf = buf;
		f.size = INT_MAX;
		va_start(ap, format);
		i = vfprintf(&f, format, ap);
		va_end(ap);
		buf[f.len] = 0;

		send(s, (uint8_t *)buf, strlen(buf));
	*/
	return i;
}

void ftpd_init ( uint8_t* src_ip, socket_cfg_t* cfg )
{
	ftp.state = FTPS_NOT_LOGIN;
	ftp.current_cmd = NO_CMD;
	ftp.dsock_mode = ACTIVE_MODE;

	s_local_ip.cVal[0] = src_ip[0];
	s_local_ip.cVal[1] = src_ip[1];
	s_local_ip.cVal[2] = src_ip[2];
	s_local_ip.cVal[3] = src_ip[3];
	s_local_port = 35000;

	strcpy ( ftp.workingdir, "/" );

}

int ftpd_run ( uint8_t* dbuf, socket_cfg_t* cfg )
{
	uint16_t size = 0;
	uint16_t blocklen, send_byte, recv_byte;
	uint32_t remain_filesize;
	uint32_t remain_datasize;

	int err;
	u8 ctrl_sock_sta, data_sock_sta;
	socklen_t addr_len;
	struct sockaddr_in remote_addr;
	struct sockaddr_in conn_addr;
	struct sockaddr_in server_addr;

#if defined(F_FILESYSTEM)
	//FILINFO fno;
#endif

	//memset(dbuf, 0, sizeof(_MAX_SS));

	ctrl_sock_sta = cfg->socket_status ( s_ctrl_socket );
	
	switch ( ctrl_sock_sta )
	{
		case SOCK_ESTABLISHED :
			if ( !s_connect_state_control )
			{
				//fsprintf(s_ctrl_socket, banner, HOSTNAME, VERSION);
				strcpy ( ftp.workingdir, "/" );
				sprintf ( ( char* ) dbuf, "220 %s FTP version %s ready.\r\n", HOSTNAME, VERSION );

				err = cfg->send ( s_ctrl_socket, ( unsigned int* ) dbuf, strlen ( ( char* ) dbuf ), 0 );

				if ( err < 0 )
				{
					cfg->close ( s_ctrl_socket );
					return err;
				}
				s_connect_state_control = 1;
			}

			size = cfg->recvfrom ( s_ctrl_socket, ( unsigned int* ) dbuf, _MAX_SS-1, 0, &conn_addr, &addr_len );

			dbuf[size] = '\0';

			if ( size == 0 )
			{
				//cfg->close ( s_ctrl_socket );

				break;
			}
			
			proc_ftpd ( ( char* ) dbuf, cfg );

			break;

		case SOCK_CLOSE_WAIT :
			cfg->close ( s_ctrl_socket );
			break;

		case SOCK_CLOSED :
			s_ctrl_socket = cfg->socket ( AF_INET, SOCK_STREAM, 0 );
			if ( s_ctrl_socket == -1 )
			{
				return -1;
			}
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr =htonl ( INADDR_ANY );
			server_addr.sin_port = IPPORT_FTP;

			err = cfg->bind ( s_ctrl_socket,  &server_addr, sizeof ( server_addr ) );
			if ( err < 0 )
			{
				return err;
			}
			break;

		case SOCK_INIT :
			err = cfg->listen ( s_ctrl_socket, 1 );
			if ( err < 0 )
			{
				return err;
			}
			s_connect_state_control = 0;

			break;
		case SOCK_LISTEN:
			break;
		default :
			break;
	}

#if 1

	data_sock_sta = cfg->socket_status ( s_data_socket );

	switch ( data_sock_sta )
	{
		case SOCK_ESTABLISHED :
			if ( !s_connect_state_data )
			{
				s_connect_state_data = 1;
			}

			switch ( ftp.current_cmd )
			{
				case LIST_CMD:
				case MLSD_CMD:
#if defined(F_FILESYSTEM)
					//scan_files ( ftp.workingdir, dbuf, ( int* ) &size );
					size = sprintf ( (char*)dbuf, "drwxr-xr-x 1 ftp ftp 0 Dec 31 2014 $Recycle.Bin\r\n-rwxr-xr-x 1 ftp ftp 512 Dec 31 2014 neo.txt\r\n" );
#endif
#if !defined(F_FILESYSTEM)
					if ( strncmp ( ftp.workingdir, "/$Recycle.Bin", sizeof ( "/$Recycle.Bin" ) ) != 0 )
					{
						size = sprintf ( dbuf, "drwxr-xr-x 1 ftp ftp 0 Dec 31 2014 $Recycle.Bin\r\n-rwxr-xr-x 1 ftp ftp 512 Dec 31 2014 neo.txt\r\n" );
					}
#endif
					size = strlen ( (char*)dbuf );
					cfg->send ( s_data_socket, ( unsigned int* ) dbuf, size, 0 );
					
					ftp.current_cmd = NO_CMD;
					cfg->disconnect ( s_data_socket );
					size = sprintf ( (char*)dbuf, "226 Successfully transferred \"%s\"\r\n", ftp.workingdir );
					cfg->send ( s_ctrl_socket, ( unsigned int* ) dbuf, size, 0 );
					break;

				case RETR_CMD:
#if defined(F_FILESYSTEM)
					ftp.fr = f_open ( & ( ftp.fil ), ( const char* ) ftp.filename, FA_READ );

					if ( ftp.fr == FR_OK )
					{
						remain_filesize = ftp.fil.fsize;
						do
						{
							memset ( dbuf, 0, _MAX_SS );

							if ( remain_filesize > _MAX_SS )
							{
								send_byte = _MAX_SS;
							}
							else
							{
								send_byte = remain_filesize;
							}

							ftp.fr = f_read ( & ( ftp.fil ), dbuf, send_byte, ( void* ) &blocklen );
							if ( ftp.fr != FR_OK )
							{
								break;
							}

							cfg->send ( s_data_socket, ( unsigned int* ) dbuf, blocklen, 0 );
							remain_filesize -= blocklen;
						}
						while ( remain_filesize != 0 );

						ftp.fr = f_close ( & ( ftp.fil ) );
					}
					else
					{
						;
					}
#else
					remain_filesize = strlen ( ftp.filename );

					do
					{
						memset ( dbuf, 0, _MAX_SS );

						blocklen = sprintf ( dbuf, "%s", ftp.filename );

						cfg->send ( s_data_socket, ( unsigned int* ) dbuf, blocklen, 0 );
						remain_filesize -= blocklen;
					}
					while ( remain_filesize != 0 );

#endif
					ftp.current_cmd = NO_CMD;
					cfg->disconnect ( s_data_socket );
					size = sprintf ( (char*)dbuf, "226 Successfully transferred \"%s\"\r\n", ftp.filename );
					cfg->send ( s_ctrl_socket, ( unsigned int* ) dbuf, size, 0 );
					break;

				case STOR_CMD:
#if defined(F_FILESYSTEM)
				/*
					ftp.fr = f_open ( & ( ftp.fil ), ( const char* ) ftp.filename, FA_CREATE_ALWAYS | FA_WRITE );

					if ( ftp.fr == FR_OK )
					{
						while ( 1 )
						{
							
							if ( ( remain_datasize = getSn_RX_RSR ( s_data_socket ) ) > 0 )
							{
								while ( 1 )
								{
									memset ( dbuf, 0, _MAX_SS );

									if ( remain_datasize > _MAX_SS )
									{
										recv_byte = _MAX_SS;
									}
									else
									{
										recv_byte = remain_datasize;
									}

									ret = recv ( s_data_socket, dbuf, recv_byte );

									ftp.fr = f_write ( & ( ftp.fil ), dbuf, ( UINT ) ret, ( void* ) &blocklen );

									remain_datasize -= blocklen;

									if ( ftp.fr != FR_OK )
									{
										break;
									}

									if ( remain_datasize <= 0 )
									{
										break;
									}
								}

								if ( ftp.fr != FR_OK )
								{
									break;
								}
								

							}
							else
							{
								if ( getSn_SR ( s_data_socket ) != SOCK_ESTABLISHED )
								{
									break;
								}
							}
						}
						ftp.fr = f_close ( & ( ftp.fil ) );
					}
					else
					{
						;
					}
*/
					//fno.fdate = (WORD)(((current_year - 1980) << 9) | (current_month << 5) | current_day);
					//fno.ftime = (WORD)((current_hour << 11) | (current_min << 5) | (current_sec >> 1));
					//f_utime((const char *)ftp.filename, &fno);
#else
					while ( 1 )
					{
						if ( ( remain_datasize = getSn_RX_RSR ( s_data_socket ) ) > 0 )
						{
							while ( 1 )
							{
								memset ( dbuf, 0, _MAX_SS );

								if ( remain_datasize > _MAX_SS )
								{
									recv_byte = _MAX_SS;
								}
								else
								{
									recv_byte = remain_datasize;
								}

								ret = recv ( s_data_socket, dbuf, recv_byte );

								remain_datasize -= ret;

								if ( remain_datasize <= 0 )
								{
									break;
								}
							}
						}
						else
						{
							if ( getSn_SR ( s_data_socket ) != SOCK_ESTABLISHED )
							{
								break;
							}
						}
					}
#endif
					ftp.current_cmd = NO_CMD;
					cfg->disconnect ( s_data_socket );
					s_data_socket = -1;
					size = sprintf ( (char*)dbuf, "226 Successfully transferred \"%s\"\r\n", ftp.filename );
					cfg->send ( s_ctrl_socket, dbuf, size, 0 );
					break;

				case NO_CMD:
				default:
					break;
			}
			break;

		case SOCK_CLOSE_WAIT :
			cfg->close ( s_data_socket );
			s_data_socket = -1;
			break;

		case SOCK_CLOSED :
			if(s_data_socket > 0)
			{
				cfg->close ( s_data_socket );
				s_data_socket = -1;
			}
			
			if ( ftp.dsock_state == DATASOCK_READY )
			{
				if ( ftp.dsock_mode == PASSIVE_MODE )
				{

					s_data_socket = cfg->socket ( AF_INET, SOCK_STREAM, 0 );
					if ( s_data_socket == -1 )
					{
						return -1;
					}
					server_addr.sin_family = AF_INET;
					server_addr.sin_addr.s_addr =htonl ( INADDR_ANY );
					server_addr.sin_port = s_local_port;

					err = cfg->bind ( s_data_socket,  &server_addr, sizeof ( server_addr ) );
					if ( err < 0 )
					{
						return err;
					}

					s_local_port++;
					if ( s_local_port > 50000 )
					{
						s_local_port = 35000;
					}
				}
				else
				{
					s_data_socket = cfg->socket ( AF_INET, SOCK_STREAM, 0 );
					if ( s_data_socket == -1 )
					{
						return -1;
					}
					server_addr.sin_family = AF_INET;
					server_addr.sin_addr.s_addr =htonl ( INADDR_ANY );
					server_addr.sin_port = IPPORT_FTPD;

					err = cfg->bind ( s_data_socket,  &server_addr, sizeof ( server_addr ) );
					if ( err < 0 )
					{
						return err;
					}
				}

				ftp.dsock_state = DATASOCK_START;
			}
			break;

		case SOCK_INIT :
			if ( ftp.dsock_mode == PASSIVE_MODE )
			{
				err = cfg->listen ( s_data_socket, 1 );
				if ( err < 0 )
				{
					return err;
				}

			}
			else
			{
				remote_addr.sin_addr.s_addr= s_remote_ip.lVal;
				remote_addr.sin_port = s_remote_port;

				cfg->connect(s_data_socket, &remote_addr, 0);
				
			}
			s_connect_state_data = 0;
			break;
		case SOCK_LISTEN:
			break;
		default :
			break;
	}
#endif

	return 0;
}

char proc_ftpd ( char* buf , socket_cfg_t* cfg)
{
	char** cmdp, *cp, *arg, *tmpstr;
	char sendbuf[200];
	int slen;
	long ret;
	struct sockaddr_in remote_addr;


	/* Translate first word to lower case */
	for ( cp = buf; *cp != ' ' && *cp != '\0'; cp++ )
	{
		*cp = tolower ( *cp );
	}

	/* Find command in table; if not present, return syntax error */
	for ( cmdp = commands; *cmdp != NULL; cmdp++ )
		if ( strncmp ( *cmdp, buf, strlen ( *cmdp ) ) == 0 )
		{
			break;
		}

	if ( *cmdp == NULL )
	{
		slen = sprintf ( sendbuf, "500 Unknown command '%s'\r\n", buf );
		cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
		return 0;
	}
	/* Allow only USER, PASS and QUIT before logging in */
	if ( ftp.state == FTPS_NOT_LOGIN )
	{
		switch ( cmdp - commands )
		{
			case USER_CMD:
			case PASS_CMD:
			case QUIT_CMD:
				break;
			default:
				slen = sprintf ( sendbuf, "530 Please log in with USER and PASS\r\n" );
				cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
				return 0;
		}
	}

	arg = &buf[strlen ( *cmdp )];
	while ( *arg == ' ' )
	{
		arg++;
	}

	/* Execute specific command */
	switch ( cmdp - commands )
	{
		case USER_CMD :
			slen = strlen ( arg );
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			strcpy ( ftp.username, arg );
			slen = sprintf ( sendbuf, "331 Enter PASS command\r\n" );
			ret = cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			if ( ret < 0 )
			{
				cfg->close ( s_ctrl_socket );
				return ret;
			}
			break;

		case PASS_CMD :
			slen = strlen ( arg );
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			ftplogin ( arg , cfg);
			break;

		case TYPE_CMD :
			slen = strlen ( arg );
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			switch ( arg[0] )
			{
				case 'A':
				case 'a':	/* Ascii */
					ftp.type = ASCII_TYPE;
					slen = sprintf ( sendbuf, "200 Type set to %s\r\n", arg );
					cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
					break;

				case 'B':
				case 'b':	/* Binary */
				case 'I':
				case 'i':	/* Image */
					ftp.type = IMAGE_TYPE;
					slen = sprintf ( sendbuf, "200 Type set to %s\r\n", arg );
					cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
					break;

				default:	/* Invalid */
					slen = sprintf ( sendbuf, "501 Unknown type \"%s\"\r\n", arg );
					cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
					break;
			}
			break;

		case FEAT_CMD :
			slen = sprintf ( sendbuf, "211-Features:\r\n MDTM\r\n REST STREAM\r\n SIZE\r\n MLST size*;type*;create*;modify*;\r\n MLSD\r\n UTF8\r\n CLNT\r\n MFMT\r\n211 END\r\n" );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			break;

		case QUIT_CMD :
			slen = sprintf ( sendbuf, "221 Goodbye!\r\n" );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			cfg->disconnect ( s_ctrl_socket );
			break;

		case RETR_CMD :
			slen = strlen ( arg );
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			if ( strlen ( ftp.workingdir ) == 1 )
			{
				sprintf ( ftp.filename, "/%s", arg );
			}
			else
			{
				sprintf ( ftp.filename, "%s/%s", ftp.workingdir, arg );
			}
			slen = sprintf ( sendbuf, "150 Opening data channel for file downloand from server of \"%s\"\r\n", ftp.filename );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			ftp.current_cmd = RETR_CMD;
			break;

		case APPE_CMD :
		case STOR_CMD:
			slen = strlen ( arg );
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			if ( strlen ( ftp.workingdir ) == 1 )
			{
				sprintf ( ftp.filename, "/%s", arg );
			}
			else
			{
				sprintf ( ftp.filename, "%s/%s", ftp.workingdir, arg );
			}
			slen = sprintf ( sendbuf, "150 Opening data channel for file upload to server of \"%s\"\r\n", ftp.filename );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			
			ftp.current_cmd = STOR_CMD;
			
			remote_addr.sin_addr.s_addr= s_remote_ip.lVal;
			remote_addr.sin_port = s_remote_port;

			cfg->connect(s_data_socket, &remote_addr, 0);
			
			s_connect_state_data = 0;
			break;

		case PORT_CMD:
			if ( pport ( arg ) == -1 )
			{
				slen = sprintf ( sendbuf, "501 Bad port syntax\r\n" );
				cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			}
			else
			{
				ftp.dsock_mode = ACTIVE_MODE;
				ftp.dsock_state = DATASOCK_READY;
				slen = sprintf ( sendbuf, "200 PORT command successful.\r\n" );
				cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			}
			break;

		case MLSD_CMD:
			slen = sprintf ( sendbuf, "150 Opening data channel for directory listing of \"%s\"\r\n", ftp.workingdir );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			ftp.current_cmd = MLSD_CMD;
			break;

		case LIST_CMD:
			slen = sprintf ( sendbuf, "150 Opening data channel for directory listing of \"%s\"\r\n", ftp.workingdir );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			ftp.current_cmd = LIST_CMD;
			break;

		case NLST_CMD:
			break;

		case SYST_CMD:
			slen = sprintf ( sendbuf, "215 UNIX emulated by WIZnet\r\n" );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			break;

		case PWD_CMD:
			slen = sprintf ( sendbuf, "257 \"%s\" is current directory.\r\n", ftp.workingdir );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			break;

		case PASV_CMD:
			slen = sprintf ( sendbuf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n", s_local_ip.cVal[0], s_local_ip.cVal[1], s_local_ip.cVal[2], s_local_ip.cVal[3], s_local_port >> 8, s_local_port & 0x00ff );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			cfg->disconnect ( s_data_socket );
			ftp.dsock_mode = PASSIVE_MODE;
			ftp.dsock_state = DATASOCK_READY;
			break;

		case SIZE_CMD:
			slen = strlen ( arg );
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			if ( slen > 3 )
			{
				tmpstr = strrchr ( arg, '/' );
				*tmpstr = 0;
#if defined(F_FILESYSTEM)
				//slen = get_filesize ( arg, tmpstr + 1 );
				slen = 0;
#else
				slen = _MAX_SS;
#endif
				/*
				if ( slen > 0 )
				{
					slen = sprintf ( sendbuf, "213 %d\r\n", slen );
				}
				else
				{
					slen = sprintf ( sendbuf, "550 File not Found\r\n" );
				}
				*/
			}
			else
			{
				slen = sprintf ( sendbuf, "550 File not Found\r\n" );
			}
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			break;

		case CWD_CMD:
			slen = strlen ( arg );
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			if ( slen > 3 )
			{
				arg[slen - 3] = 0x00;
				tmpstr = strrchr ( arg, '/' );
				*tmpstr = 0;
#if defined(F_FILESYSTEM)
				//slen = get_filesize ( arg, tmpstr + 1 );
#else
				slen = 0;
#endif
				*tmpstr = '/';
				if ( slen == 0 )
				{
					slen = sprintf ( sendbuf, "213 %d\r\n", slen );
					strcpy ( ftp.workingdir, arg );
					slen = sprintf ( sendbuf, "250 CWD successful. \"%s\" is current directory.\r\n", ftp.workingdir );
				}
				else
				{
					slen = sprintf ( sendbuf, "550 CWD failed. \"%s\"\r\n", arg );
				}
			}
			else
			{
				strcpy ( ftp.workingdir, arg );
				slen = sprintf ( sendbuf, "250 CWD successful. \"%s\" is current directory.\r\n", ftp.workingdir );
			}
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			break;

		case MKD_CMD:
			slen = strlen ( arg );
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
#if defined(F_FILESYSTEM)
			if ( f_mkdir ( arg ) != 0 )
			{
				slen = sprintf ( sendbuf, "550 Can't create directory. \"%s\"\r\n", arg );
			}
			else
			{
				slen = sprintf ( sendbuf, "257 MKD command successful. \"%s\"\r\n", arg );
				//strcpy(ftp.workingdir, arg);
			}
#else
			slen = sprintf ( sendbuf, "550 Can't create directory. Permission denied\r\n" );
#endif
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			break;

		case DELE_CMD:
			slen = strlen ( arg );
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
#if defined(F_FILESYSTEM)
			if ( f_unlink ( arg ) != 0 )
			{
				slen = sprintf ( sendbuf, "550 Could not delete. \"%s\"\r\n", arg );
			}
			else
			{
				slen = sprintf ( sendbuf, "250 Deleted. \"%s\"\r\n", arg );
			}
#else
			slen = sprintf ( sendbuf, "550 Could not delete. Permission denied\r\n" );
#endif
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			break;

		case XCWD_CMD:
		case XPWD_CMD:
		case ACCT_CMD:
		case XMKD_CMD:
		case XRMD_CMD:
		case RMD_CMD:
		case STRU_CMD:
		case MODE_CMD:
		case XMD5_CMD:
			slen = sprintf ( sendbuf, "502 Command does not implemented yet.\r\n" );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			break;

		default:	/* Invalid */
			slen = sprintf ( sendbuf, "500 Unknown command \'%s\'\r\n", arg );
			cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
			break;
	}

	return 1;
}


char ftplogin ( char* pass ,socket_cfg_t* cfg)
{
	char sendbuf[100];
	int slen = 0;
	
	slen = sprintf ( sendbuf, "230 Logged on\r\n" );
	cfg->send ( s_ctrl_socket, ( unsigned int* ) sendbuf, slen, 0 );
	ftp.state = FTPS_LOGIN;

	return 1;
}

int pport ( char* arg )
{
	int i;
	char* tok=0;

	for ( i = 0; i < 4; i++ )
	{
		if ( i==0 )
		{
			tok = strtok ( arg,",\r\n" );
		}
		else
		{
			tok = strtok ( NULL,"," );
		}
		s_remote_ip.cVal[i] = ( uint8_t ) ATOI ( tok, 10 );
		if ( !tok )
		{
			return -1;
		}
	}
	s_remote_port = 0;
	for ( i = 0; i < 2; i++ )
	{
		tok = strtok ( NULL,",\r\n" );
		s_remote_port <<= 8;
		s_remote_port += ATOI ( tok, 10 );
		if ( !tok )
		{
			return -1;
		}
	}

	return 0;
}

#if defined(F_FILESYSTEM)
void print_filedsc ( FIL* fil )
{
	/*
	#if defined(_FTP_DEBUG_)
		printf ( "File System pointer : %08X\r\n", fil->fs );
		printf ( "File System mount ID : %d\r\n", fil->id );
		printf ( "File status flag : %08X\r\n", fil->flag );
		printf ( "File System pads : %08X\r\n", fil->err );
		printf ( "File read write pointer : %08X\r\n", fil->fptr );
		printf ( "File size : %08X\r\n", fil->fsize );
		printf ( "File start cluster : %08X\r\n", fil->sclust );
		printf ( "current cluster : %08X\r\n", fil->clust );
		printf ( "current data sector : %08X\r\n", fil->dsect );
		printf ( "dir entry sector : %08X\r\n", fil->dir_sect );
		printf ( "dir entry pointer : %08X\r\n", fil->dir_ptr );
	#endif
	*/
}
#endif

