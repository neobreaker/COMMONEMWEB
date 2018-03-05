#include "stm32f10x.h"
#include "w5500_socket.h"
#include "ucos_ii.h"
#include "task_socket.h"
#include <string.h>

extern OS_EVENT* mbox_sock_rcv[W5500_SOCKET_NUM];

static u8 s_socket_usage = 0;
static u8 s_socket_type[8];

u8 w5500_send_buffer[2048];

int socket(int domain, int type, int protocol)
{
    int i;

    for(i = 0; i < 8; i++)
    {
        if((s_socket_usage & (1<<i)) == 0)
        {
            s_socket_usage |= (1<<i);
            break;
        }
    }
    if(i >= 8)
        return -1;

    switch (type)
    {
        case SOCK_RAW:
            break;
        case SOCK_DGRAM:
            W5500_Socket_UDP(i);
            s_socket_type[i] = SOCK_DGRAM;
            break;
        case SOCK_STREAM:
			W5500_Socket_TCP(i);
			s_socket_type[i] = SOCK_STREAM;
            break;
        default:
            return -1;
    }

    return i;
}

int bind(int s, const struct sockaddr_in *name, socklen_t namelen)
{
    int ret = 0;
    if(!((namelen == sizeof(struct sockaddr_in)
          && (name->sin_family) == AF_INET) ))
        return -1;

    W5500_Socket_Init(s, name->sin_port);

    switch(s_socket_type[s])
    {
        case SOCK_RAW:
            break;
        case SOCK_DGRAM:
            if(!W5500_Socket_UDP_Open(s))
                ret = -1;
            break;
        case SOCK_STREAM:
			if(!W5500_Socket_TCP_Open(s))
				return -1;
            break;
        default:
            return -1;
    }

    return ret;
}

int listen(int s, int backlog)
{
	return W5500_Socket_TCP_Listen(s);
}

int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
	return W5500_Socket_TCP_Connect(s);
}

int sendto(int s, const void *data, u32 size, int flags,
           const struct sockaddr_in *to, socklen_t tolen)
{

	memcpy(w5500_send_buffer, data, size);
	
    Write_SOCK_Data_Buffer(s, (u8*)w5500_send_buffer, size, (u8*)&(to->sin_addr), to->sin_port);

    return size;
}

//Be careful, only use for tcp
int send(int s, const void *data, u32 size, int flags)
{
	memcpy(w5500_send_buffer, data, size);
	
    Write_SOCK_Data_Buffer(s, (u8*)w5500_send_buffer, size, NULL, 0);

    return size;
}

int recvfrom(int s, void *mem, u32 len, int flags, struct sockaddr_in *from, socklen_t *fromlen)
{
    int rcv_len = 0;
    u8 err;
    rcv_pack_t *mail = NULL;

    mail = OSMboxPend(mbox_sock_rcv[s], 0, &err);

    if(err == OS_ERR_NONE)
    {
		from->sin_addr.s_addr = mail->remote_ip;
		from->sin_port = mail->remote_port;
		
        rcv_len = (mail->len > len) ? len : mail->len;

        memcpy(mem, mail->data, rcv_len);
    }

    return rcv_len;
}

u8 socket_status(int s)
{
	return getSn_SR(s);
}

/**
 * Convert an u16_t from host- to network byte order.
 *
 * @param n u16_t in host byte order
 * @return n in network byte order
 */
u16_t htons(u16_t n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

/**
 * Convert an u16_t from network- to host byte order.
 *
 * @param n u16_t in network byte order
 * @return n in host byte order
 */
u16_t ntohs(u16_t n)
{
    return htons(n);
}

/**
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
u32_t htonl(u32_t n)
{
    return ((n & 0xff) << 24) |
           ((n & 0xff00) << 8) |
           ((n & 0xff0000UL) >> 8) |
           ((n & 0xff000000UL) >> 24);
}

/**
 * Convert an u32_t from network- to host byte order.
 *
 * @param n u32_t in network byte order
 * @return n in host byte order
 */
u32_t ntohl(u32_t n)
{
    return  htonl(n);
}

