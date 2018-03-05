#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_startup.h"
#include "lwip/err.h"
#include "task_httpd.h"
#include "httpd.h"


void task_httpd(void *p_arg)
{

	//httpd_init();
	
	while(1)
	{
		OSTimeDly(1000);
	}
}

