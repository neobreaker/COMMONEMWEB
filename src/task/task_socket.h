#ifndef __TASK_SOCKET_H__
#define __TASK_SOCKET_H__

typedef struct
{
	u8* data;
	u32 len;
	u32 remote_ip;
	u16 remote_port;
	
}rcv_pack_t;

void task_socket(void *p_arg);

#endif
