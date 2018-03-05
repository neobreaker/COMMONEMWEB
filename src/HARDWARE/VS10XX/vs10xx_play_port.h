#ifndef __VS10XX_PLAY_PORT_H__
#define __VS10XX_PLAY_PORT_H__

#include "vs10xx_core.h"

#define  VS10XX_PLAY_DMA_CHx 			DMA2_Channel2

void vs10xx_play_cfg_setup(vs10xx_cfg_t* g_vs10xx_cfg);
void vs10xx_play_dma_enable(u16 cndtr);

#endif
