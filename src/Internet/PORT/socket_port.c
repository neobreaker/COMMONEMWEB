#include "socket_port.h"

int socket_cfg_init(socket_cfg_t *cfg)
{
	cfg->socket = socket;
	cfg->bind = bind;
	cfg->listen = listen;
	cfg->connect = connect;
	cfg->sendto = sendto;
	cfg->send =send;
	cfg->recvfrom = recvfrom;
	cfg->socket_status = socket_status;
    cfg->disconnect = disconnect;
    cfg->close = close;
    cfg->malloc = pvPortMalloc;
    cfg->free = vPortFree;
	return 0;
}

