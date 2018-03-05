#ifndef  _APP_CFG_H_
#define  _APP_CFG_H_

/* task priority */
#define STARTUP_TASK_PRIO                      4

#define TASK_SOCKET_PRIO					   14
#define TASK_TCPSERVER_PRIO                    15
#define TASK_UDPSERVER_PRIO                    16
#define TASK_PLAY_PRIO                    	   17
#define TASK_UDPCLIENT_PRIO                    18


/* task stack size */
#define STARTUP_TASK_STK_SIZE                  80
#define TASK_TCPSERVER_STK_SIZE         	   256
#define TASK_UDPSERVER_STK_SIZE         	   256
#define TASK_UDPCLIENT_STK_SIZE         	   256
#define TASK_PLAY_STK_SIZE         	   		   256
#define TASK_SOCKET_STK_SIZE         	   	   256

/* interrupt priority */

#endif

