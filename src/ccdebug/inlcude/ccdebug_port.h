#ifndef __CCDEBUG_PORT_H__
#define __CCDEBUG_PORT_H__

#include "stm32f10x.h"

#ifndef CCDBG_PLATFORM_ASSERT
#define CCDBG_PLATFORM_ASSERT(x) \
    do \
    {   printf("Assertion \"%s\" failed at line %d in %s\r\n", x, __LINE__, __FILE__); \
    } while(0)
#endif


#ifndef CCDBG_PLATFORM_DIAG
#define CCDBG_PLATFORM_DIAG(x) do {printf("%s", x);} while(0)
#endif


void ccdebug_port_init(u32 baud);

#endif

