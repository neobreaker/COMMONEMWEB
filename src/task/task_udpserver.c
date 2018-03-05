#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_startup.h"
#include "lib_mem.h"
#include "task_udpserver.h"
#include "w5500_socket.h"

extern OS_EVENT* sem_vs1053async;
extern OS_EVENT* sem_vs1053_play_async;

struct sockaddr_in g_remote_sin;

u8 is_line_established = 0;     //通讯连接是否建立
rev_buffer_t g_pbuff;

static void rev_buffer_init()
{

    g_pbuff.data = pvPortMalloc(RCV_BUFFER_SIZE);
    g_pbuff.len = 0;


}


void task_udpserver(void *p_arg)
{

    int err;
    struct sockaddr_in sin;
    socklen_t sin_len;
    int sock_fd;
    int ret = 0;

    rev_buffer_init();


    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1)
    {
        return ;
    }

    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=INADDR_ANY;
    sin.sin_port=SRC_RCV_PORT;
    sin_len = sizeof(sin);

    err = bind(sock_fd, &sin, sizeof(sin));
    if (err < 0)
    {
        return ;
    }

    while(1)
    {

        g_pbuff.len = recvfrom(sock_fd, g_pbuff.data, RCV_BUFFER_SIZE, 0, &g_remote_sin, &sin_len);

        ret = parse_AT(g_pbuff.data, RCV_BUFFER_SIZE);
        if(ret)
        {
            g_pbuff.len = 0;
            if(is_line_established)
            {
                OSSemPost(sem_vs1053async);
                OSSemPost(sem_vs1053_play_async);
            }
        }

		if(g_pbuff.len > 0)
			OSTimeDly(1);

    }

}

//return 0 : NOT AT  1: AT
int parse_AT(u8* buffer, int len)
{
    u8 cmd = 0;
    if(buffer[0] == 'A' && buffer[1] == 'T')
    {
        cmd = buffer[2];

        switch(cmd)
        {
            case AT_LINE_EATABLISH:
                is_line_established = 1;
                break;
            case AT_LINE_SHUTDOWN:
                is_line_established = 0;
                break;
        }
        return 1;
    }
    return 0;
}


