#include "ucos_ii.h"
#include "stm32f10x.h"
#include "mysys.h"
#include "task_startup.h"
#include "sram.h"
#include "stdio.h"
#include "lib_mem.h"
#include "array.h"
#include "delay.h"
#include "ccdebug.h"
//#include "vs10xx_port.h"
//#include "vs10xx_play_port.h"
#include "w5500_port.h"

static OS_STK startup_task_stk[STARTUP_TASK_STK_SIZE];

//OS_EVENT* sem_vs1053async;          //TEST
//OS_EVENT* sem_vs1053_play_async;
OS_EVENT* mbox_sock_rcv[W5500_SOCKET_NUM];
OS_EVENT* sem_w5500;
OS_EVENT* sem_w5500_dma;

//vs10xx_cfg_t g_vs10xx_rec_cfg;
//vs10xx_cfg_t g_vs10xx_play_cfg;

w5500_cfg_t g_w5500_cfg;
netchard_dev_t g_netchard_dev;

void load_netcard_dev()
{
	g_netchard_dev.gateway[0] = 192;
	g_netchard_dev.gateway[1] = 168;
	g_netchard_dev.gateway[2] = 2;
	g_netchard_dev.gateway[3] = 1;

	g_netchard_dev.netmask[0] = 255;
	g_netchard_dev.netmask[1] = 255;
	g_netchard_dev.netmask[2] = 255;
	g_netchard_dev.netmask[3] = 0;

	g_netchard_dev.mac[0] = 0x0c;
	g_netchard_dev.mac[1] = 0x29;
	g_netchard_dev.mac[2] = 0xab;
	g_netchard_dev.mac[3] = 0x7c;
	g_netchard_dev.mac[4] = 0x00;
	g_netchard_dev.mac[5] = 0x01;

	g_netchard_dev.ip[0] = 192;
	g_netchard_dev.ip[1] = 168;
	g_netchard_dev.ip[2] = 2;
	g_netchard_dev.ip[3] = 60;
}

void bsp_init()
{
	NVIC_Configuration();
	delay_init();
	FSMC_SRAM_Init();
	ccdebug_port_init ( 9600 );
//    vs10xx_cfg_setup(&g_vs10xx_rec_cfg);
//    vs10xx_play_cfg_setup(&g_vs10xx_play_cfg);

	w5500_cfg_setup ( &g_w5500_cfg );
	load_netcard_dev();
	W5500_Hardware_Reset ( &g_w5500_cfg );
	W5500_Init ( &g_w5500_cfg, &g_netchard_dev );
}

int main ( void )
{
	u8 i = 0;
	bsp_init();

	delay_ms ( 3000 );  /* wait power stable */

	OSInit();

	CC_DEBUGF ( CC_DBG_ON | CC_DBG_LEVEL_WARNING, "system booting\n" );
	/*
	    sem_vs1053async         = OSSemCreate(0);
	    sem_vs1053_play_async   = OSSemCreate(0);
	*/
	sem_w5500				= OSSemCreate ( 1 );
	sem_w5500_dma			= OSSemCreate ( 0 );

	for ( i = 0; i < W5500_SOCKET_NUM; i++ )
	{
		mbox_sock_rcv[i] = OSMboxCreate ( 0 );
	}

	OSTaskCreate ( startup_task, ( void* ) 0,
	               &startup_task_stk[STARTUP_TASK_STK_SIZE - 1],
	               STARTUP_TASK_PRIO );
	OSStart();

	CC_DEBUGF ( CC_DBG_ON | CC_DBG_LEVEL_WARNING, "system shutdonwn\n" );   /* should never run here! */

	return 0;
}
