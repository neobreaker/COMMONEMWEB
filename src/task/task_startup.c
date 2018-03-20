#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_startup.h"

static OS_STK task_tcpserver_stk[TASK_TCPSERVER_STK_SIZE];
static OS_STK task_socket_stk[TASK_SOCKET_STK_SIZE];
//static OS_STK task_ftpd_stk[TASK_FTPD_STK_SIZE];

extern void task_tcpserver(void *p_arg);
extern void task_socket(void *p_arg);
//extern void task_ftpd ( void* p_arg );

static void systick_init(void)
{
    RCC_ClocksTypeDef rcc_clocks;
    RCC_GetClocksFreq(&rcc_clocks);
    SysTick_Config(rcc_clocks.HCLK_Frequency / OS_TICKS_PER_SEC);
}

void startup_task(void *p_arg)
{
    INT8U err = 0;
    systick_init();     /* Initialize the SysTick. */

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();      /* Determine CPU capacity. */
#endif
    /* TODO: create application tasks here */


    
    err = OSTaskCreate(task_tcpserver, (void *)0,
                       &task_tcpserver_stk[TASK_TCPSERVER_STK_SIZE-1], TASK_TCPSERVER_PRIO);
	
	err |= OSTaskCreate(task_socket, (void *)0,
                        &task_socket_stk[TASK_SOCKET_STK_SIZE-1], TASK_SOCKET_PRIO);
/*
	err |= OSTaskCreate(task_ftpd, (void *)0,
                       &task_ftpd_stk[TASK_FTPD_STK_SIZE-1], TASK_FTPD_PRIO);
*/
    if (OS_ERR_NONE != err)
        while(1)
            ;
    OSTaskDel(OS_PRIO_SELF);
}

