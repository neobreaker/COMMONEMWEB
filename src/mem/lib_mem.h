#ifndef _LIB_MEM_H
#define _LIB_MEM_H

#include <stdlib.h>
#include <string.h>

void *pvPortMalloc( size_t xWantedSize );
void *pvPortCalloc(size_t n, size_t xWantedSize );
void vPortFree( void *pv );
void pvPortMemDeinit(void);
size_t xPortGetFreeHeapSize( void );


#endif
