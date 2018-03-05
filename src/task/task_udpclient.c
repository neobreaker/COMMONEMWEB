#include "ucos_ii.h"
#include "stm32f10x.h"
#include "task_startup.h"
#include "lib_mem.h"
#include "task_udpclient.h"
#include "vs10xx_port.h"
#include "wav.h"
#include "w5500_core.h"
#include "w5500_socket.h"

extern OS_EVENT* sem_vs1053async;
extern u8 is_line_established;
extern struct sockaddr_in g_remote_sin;
extern vs10xx_cfg_t g_vs10xx_rec_cfg;

static u32 time_stamp = 0, time_elapse = 0;

void task_udpclient(void *p_arg)
{
	
    int err = 0;
    u8 _err = 0;
    struct sockaddr_in sin;
    socklen_t sin_len;
    int sock_fd;
    u8* snddata;
    int i = 0;
    u16 w = 0;
    u32 idx = 0;
	u32 cnt = 0;
	WaveHeaderStruct wav_header;

    snddata = pvPortMalloc(512);
    memset(snddata, 0x5a, 512);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1)
    {
        return ;
    }

    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=INADDR_ANY;
    sin.sin_port=SRC_SND_PORT;
    sin_len = sizeof(sin);

    err = bind(sock_fd, &sin, sizeof(sin));
    if (err < 0)
    {
        return ;
    }

	recoder_enter_rec_mode(&g_vs10xx_rec_cfg, 1024*4);
	while(VS_RD_Reg(&g_vs10xx_rec_cfg, SPI_HDAT1)>>8);
    while(1)
    {
		//W5500_Interrupt_Process();
		//OSTimeDly(200);
		
        OSSemPend(sem_vs1053async, 0, &_err);
		
        if(_err == OS_ERR_NONE)
        {
            sin.sin_addr.s_addr = g_remote_sin.sin_addr.s_addr;
            sin.sin_port = ntohs(g_remote_sin.sin_port)-1;
            //sin.sin_addr.s_addr = 0x4602a8c0;
			//sin.sin_port = 50000;
            time_stamp = OSTimeGet();
			
			cnt = 500;
			
			wav_header_init(&wav_header, cnt*512+36, cnt*512);
			
			sendto(sock_fd, &wav_header, sizeof(wav_header), 0, &sin, sizeof(sin));
			
            while(1)
            {

				w=VS_RD_Reg(&g_vs10xx_rec_cfg, SPI_HDAT1);
                if((w>=256)&&(w<896))
                {
                    idx=0;
					
                    while(idx<512)  //一次读取512字节
                    {
                        w=VS_RD_Reg(&g_vs10xx_rec_cfg, SPI_HDAT0);
                        snddata[idx++]=w&0XFF;
                        snddata[idx++]=w>>8;
                    }
                    
                    sendto(sock_fd, snddata, 512, 0, &sin, sizeof(sin));
                    
                }
				
				if(!is_line_established)		// 通讯结束
					break;
            }
            time_elapse = OSTimeGet() - time_stamp;
            time_elapse = 0;
        }
        
    }
	
}


