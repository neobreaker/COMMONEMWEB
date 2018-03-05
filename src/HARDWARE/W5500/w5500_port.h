#ifndef _W5500_PORT_H_
#define _W5500_PORT_H_

#include "w5500_core.h"
#include "mysys.h"

#define W5500_DMA_CHx		DMA1_Channel5

void w5500_cfg_setup(w5500_cfg_t* cfg);
void w5500_dma_enable(u16 cndtr);

#endif

