#ifndef __TASK_UDPSERVER_H__
#define __TASK_UDPSERVER_H__

#define SRC_RCV_PORT			50000



#define AT_LINE_EATABLISH		0X31
#define AT_LINE_SHUTDOWN		0X32

#define RCV_BUFFER_SIZE			2048
#define RCV_BUFFER_NUM			3

typedef struct 
{
	u8* data;
	int len;
}rev_buffer_t;

void task_udpserver(void *p_arg);
int parse_AT(u8* buffer, int len);

#endif
