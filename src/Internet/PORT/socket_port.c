#include "socket_port.h"


int socket_port_init(socket_port_t *port)
{
	port->socket = socket;
	port->bind = bind;
	port->listen = listen;
	port->connect = connect;
	port->sendto = sendto;
	port->send =send;
	port->recvfrom = recvfrom;
	port->socket_status = socket_status;
	return 0;
}

