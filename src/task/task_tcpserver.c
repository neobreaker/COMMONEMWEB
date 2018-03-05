#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_startup.h"
#include "w5500_socket.h"
#include "socket_port.h"
#include "httpServer.h"
#include "lib_mem.h"

static u8* tx_buffer;
static u8* rx_buffer;

void task_tcpserver(void *p_arg)
{
	int s;
	int err;
	socket_port_t port;
	struct sockaddr_in server_addr;

	tx_buffer = pvPortMalloc(1024);
	rx_buffer = pvPortMalloc(1024);
	
	socket_port_init(&port);

	httpServer_init(tx_buffer, rx_buffer);

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) 
	{  
        return ;
    }

	server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr =htonl(INADDR_ANY);  
    server_addr.sin_port = 80;

	err = bind(s,  &server_addr, sizeof(server_addr));  
    if (err < 0) 
	{  
        return ;  
    }

	while(1)
	{
		httpServer_run(&s, &port);
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


