#ifndef __USERHANDLER_H__
#define __USERHANDLER_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "httpParser.h"

int8_t set_diostate ( uint8_t* uri );
uint8_t predefined_set_cgi_processor ( uint8_t* uri_name, uint8_t* uri, uint8_t* buf, uint16_t* len );
void make_json_ain ( uint8_t* buf, uint16_t* len, uint8_t pin );
uint8_t predefined_get_cgi_processor ( uint8_t* uri_name, uint8_t* buf, uint16_t* len );

#endif
