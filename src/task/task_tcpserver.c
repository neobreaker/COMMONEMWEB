#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_startup.h"
#include "w5500_socket.h"
#include "socket_port.h"
#include "httpServer.h"
#include "lib_mem.h"
#include "webpage.h"
#include "w5500_port.h"

static u8* tx_buffer;
static u8* rx_buffer;

extern w5500_cfg_t g_w5500_cfg;

static u8 mr;

void task_tcpserver ( void* p_arg )
{
	int s = -1;

	socket_cfg_t cfg;

	tx_buffer = pvPortMalloc ( 1024 );
	rx_buffer = pvPortMalloc ( 1024 );

	socket_cfg_init ( &cfg );

	httpServer_init ( tx_buffer, rx_buffer );

	reg_httpServer_webContent ( ( uint8_t* ) "index.html", ( uint8_t* ) index_page, &cfg );

	// Example #1
	reg_httpServer_webContent ( ( uint8_t* ) "dio.html", ( uint8_t* ) dio_page, &cfg );				// dio.html 		: Digital I/O control example page
	reg_httpServer_webContent ( ( uint8_t* ) "dio.js", ( uint8_t* ) wiz550web_dio_js, &cfg );			// dio.js 			: JavaScript for digital I/O control 	(+ ajax.js)

	// Example #2
	reg_httpServer_webContent ( ( uint8_t* ) "ain.html", ( uint8_t* ) ain_page ,&cfg);					// ain.html 		: Analog input monitor example page
	reg_httpServer_webContent ( ( uint8_t* ) "ain.js", ( uint8_t* ) wiz550web_ain_js ,&cfg);

	// AJAX JavaScript functions
	reg_httpServer_webContent ( ( uint8_t* ) "ajax.js", ( uint8_t* ) wiz550web_ajax_js,&cfg );			// ajax.js			: JavaScript for AJAX request transfer


	while ( 1 )
	{
		//mr = Read_W5500_1Byte(&g_w5500_cfg, MR);
		httpServer_run ( &s, &cfg );
	}
}






































/*
void task_tcpserver(void *p_arg)
{
	int err;
	struct sockaddr_in server_addr;
    struct sockaddr_in conn_addr;
	int sock_fd;

	socklen_t addr_len;
	int length;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
	{
        return ;
    }

	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr =htonl(INADDR_ANY);
    server_addr.sin_port = 80;

	err = bind(sock_fd,  &server_addr, sizeof(server_addr));
    if (err < 0)
	{
        return ;
    }

	err = listen(sock_fd, 1);
    if (err < 0) {
        return ;
    }

	//sock_conn = accept(sock_fd, (struct sockaddr *)&conn_addr, &addr_len);

	while(1)
	{

		length = recvfrom(sock_fd, (unsigned int *)data_buffer, 1024, 0, &conn_addr, &addr_len);
		send(sock_fd, (unsigned int *)data_buffer, length, 0);
	}
}
*/


