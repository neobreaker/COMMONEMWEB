#include "task_ftpd.h"
#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_startup.h"
#include "w5500_socket.h"
#include "socket_port.h"
#include "w5500_port.h"
#include "ftpd.h"

extern netchard_dev_t g_netchard_dev;

void task_ftpd ( void* p_arg )
{
	socket_cfg_t cfg;
	uint8_t* dbuf;

	dbuf = pvPortMalloc(512);

	socket_cfg_init ( &cfg );

	ftpd_init ( g_netchard_dev.ip, &cfg );

	while ( 1 )
	{
		//OSTimeDly ( 200 );
		ftpd_run (dbuf, &cfg );
	}
}


