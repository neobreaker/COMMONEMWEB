#include "userhandler.h"

int8_t set_diostate ( uint8_t* uri )
{
	uint8_t* param;
	uint8_t pin = 0, val = 0;

	if ( ( param = get_http_param_value ( ( char* ) uri, "pin" ) ) ) // GPIO; D0 ~ D15
	{
		pin = ( uint8_t ) ATOI ( param, 10 );
		if ( pin > 15 )
		{
			return -1;
		}

		if ( ( param = get_http_param_value ( ( char* ) uri, "val" ) ) ) // State; high(on)/low(off)
		{
			val = ( uint8_t ) ATOI ( param, 10 );
			//if(val > On) val = On;
		}
		/*
				if(val == On) 		Chip_GPIO_SetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin], true); 	// High
				else				Chip_GPIO_SetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin], false);	// Low*/
	}

	return pin;
}


uint8_t predefined_set_cgi_processor ( uint8_t* uri_name, uint8_t* uri, uint8_t* buf, uint16_t* len )
{
	uint8_t ret = 1;	// ret = '1' means 'uri_name' matched
	uint8_t val = 0;

	if ( strcmp ( ( const char* ) uri_name, "todo.cgi" ) == 0 )
	{
		// to do
		;//val = todo(uri);
		//*len = sprintf((char *)buf, "%d", val);
	}
	// Digital I/O; dio_s, dio_d
	else if ( strcmp ( ( const char* ) uri_name, "set_diodir.cgi" ) == 0 )
	{
		//val = set_diodir(uri);
		*len = sprintf ( ( char* ) buf, "%d", val );
	}
	else if ( strcmp ( ( const char* ) uri_name, "set_diostate.cgi" ) == 0 )
	{
		val = set_diostate ( uri );
		*len = sprintf ( ( char* ) buf, "%d", val );
	}
	else
	{
		ret = 0;
	}

	return ret;
}

void make_json_ain ( uint8_t* buf, uint16_t* len, uint8_t pin )
{
	*len = sprintf ( ( char* ) buf, "AinCallback({\"ain_p\":\"%d\",\
											\"ain_v\":\"%d\"\
											});",
	                 pin,					// ADC input pin number
	                 pin						// ADC input value
	               );
}

uint8_t predefined_get_cgi_processor ( uint8_t* uri_name, uint8_t* buf, uint16_t* len )
{
	uint8_t ret = 1;	// ret = 1 means 'uri_name' matched
	uint8_t cgibuf[14] = {0, };
	int8_t cgi_dio = -1;
	int8_t cgi_ain = -1;

	uint8_t i;

	if ( strcmp ( ( const char* ) uri_name, "todo.cgi" ) == 0 )
	{
		// to do
		;//make_json_todo(buf, len);
	}
	else if ( strcmp ( ( const char* ) uri_name, "get_netinfo.cgi" ) == 0 )
	{
		//make_json_netinfo(buf, len);
	}
	else
	{
		/*
		// get_dio0.cgi ~ get_dio15.cgi
		for(i = 0; i < DIOn; i++)
		{
			memset(cgibuf, 0x00, 14);
			sprintf((char *)cgibuf, "get_dio%d.cgi", i);
			if(strcmp((const char *)uri_name, (const char *)cgibuf) == 0)
			{
				make_json_dio(buf, len, i);
				cgi_dio = i;
				break;
			}
		}
		*/

		if ( cgi_dio < 0 )
		{
			// get_ain0.cgi ~ get_ain5.cgi (A0 - A5), get_ain6.cgi for on-board potentiometer / Temp.Sensor
			for ( i = 0; i < 7; i++ )
			{
				memset ( cgibuf, 0x00, 14 );
				sprintf ( ( char* ) cgibuf, "get_ain%d.cgi", i );
				if ( strcmp ( ( const char* ) uri_name, ( const char* ) cgibuf ) == 0 )
				{
					make_json_ain ( buf, len, i );
					cgi_ain = i;
					break;
				}
			}
		}

		if ( ( cgi_dio < 0 ) && ( cgi_ain < 0 ) )
		{
			ret = 0;
		}
	}

	return ret;
}

