#ifndef __SOCKET_PORT_H__
#define __SOCKET_PORT_H__

#include "stm32f10x.h"
#include "w5500_socket.h"
#include "lib_mem.h"

typedef struct
{
	int (*socket)(int domain, int type, int protocol);
	int (*bind)(int s, const struct sockaddr_in *name, socklen_t namelen);
	int (*listen)(int s, int backlog);
	int (*connect)(int s, const struct sockaddr *name, socklen_t namelen);
	int (*sendto)(int s, const void *data, u32 size, int flags,
	    const struct sockaddr_in *to, socklen_t tolen);
	int (*send)(int s, const void *data, u32 size, int flags);
	int (*recvfrom)(int s, void *mem, u32 len, int flags, struct sockaddr_in *from, socklen_t *fromlen);
    u8  (*socket_status)(int);
    int8_t (*disconnect)(int s);
    int8_t (*close)(int s);
    void* (*malloc)( size_t size );
    void (*free)( void *pv );
} socket_cfg_t;

int socket_cfg_init(socket_cfg_t *cfg);

#endif


