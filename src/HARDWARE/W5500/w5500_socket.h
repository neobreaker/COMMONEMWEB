#ifndef _W5500_SOCKET_H_
#define _W5500_SOCKET_H_

#include "w5500_core.h"

/* Socket Domain */
#define AF_UNSPEC       0
#define AF_INET         2
#define PF_INET         AF_INET
#define PF_UNSPEC       AF_UNSPEC

/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

/* Socket protocol */
#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_UDPLITE 136

/** 255.255.255.255 */
#define IPADDR_NONE         ((u32)0xffffffffUL)
/** 127.0.0.1 */
#define IPADDR_LOOPBACK     ((u32)0x7f000001UL)
/** 0.0.0.0 */
#define IPADDR_ANY          ((u32)0x00000000UL)
/** 255.255.255.255 */
#define IPADDR_BROADCAST    ((u32)0xffffffffUL)

/** 255.255.255.255 */
#define INADDR_NONE         IPADDR_NONE
/** 127.0.0.1 */
#define INADDR_LOOPBACK     IPADDR_LOOPBACK
/** 0.0.0.0 */
#define INADDR_ANY          IPADDR_ANY
/** 255.255.255.255 */
#define INADDR_BROADCAST    IPADDR_BROADCAST


//定义与平台无关的数据类型
typedef unsigned   char    u8_t;  	//无符号8位整数  
typedef signed     char    s8_t;   	//有符号8位整数 
typedef unsigned   short   u16_t;  	//无符号16位整数
typedef signed     short   s16_t;   //有符号16位整数
typedef unsigned   long    u32_t;   //无符号32位整数
typedef signed     long    s32_t;   //有符号32位整数
typedef u32_t mem_ptr_t;            //内存地址型数据
typedef int sys_prot_t;				//临界保护型数据

typedef u32_t socklen_t;

/** For compatibility with BSD code */
struct in_addr {
  u32 s_addr;
};

/* members are in network byte order */
struct sockaddr_in {
  u8 sin_len;
  u8 sin_family;
  u16 sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct sockaddr {
  u8 sa_len;
  u8 sa_family;
  char sa_data[14];
};


int socket(int domain, int type, int protocol);
int bind(int s, const struct sockaddr_in *name, socklen_t namelen);
int sendto(int s, const void *data, u32 size, int flags,
            const struct sockaddr_in *to, socklen_t tolen);
int send(int s, const void *data, u32 size, int flags);
int recvfrom(int s, void *mem, u32 len, int flags, struct 
			sockaddr_in *from, socklen_t *fromlen);
int listen(int s, int backlog);
int connect(int s, const struct sockaddr *name, socklen_t namelen);
u8 socket_status(int s);
int8_t disconnect(int s);
int8_t close(int s);

u16_t htons(u16_t n);
u16_t ntohs(u16_t n);
u32_t htonl(u32_t n);
u32_t ntohl(u32_t n);

#endif

