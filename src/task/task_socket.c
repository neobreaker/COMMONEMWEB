#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_socket.h"
#include "w5500_socket.h"
#include "lib_mem.h"

extern OS_EVENT* mbox_sock_rcv[W5500_SOCKET_NUM];

rcv_pack_t g_pack;

void task_socket(void *p_arg)
{
	u8 i = 0;
	g_pack.data = pvPortMalloc(2048);
	
	if(!g_pack.data)
		return ;
	
	while(1)
	{
		for (; i < W5500_SOCKET_NUM; i++)
		{
			g_pack.len = Read_SOCK_Data_Buffer(i, g_pack.data, &g_pack.remote_ip, &g_pack.remote_port);
			if(g_pack.len > 0)
			{
				OSMboxPost(mbox_sock_rcv[i], &g_pack);
				break;
			}
		}
		i%=W5500_SOCKET_NUM;
		OSTimeDly(4);
		
	}
	
}


