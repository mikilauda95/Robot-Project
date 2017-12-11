#include <stdio.h>
#include <stdlib.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"
// WIN32 /////////////////////////////////////////
#ifdef __WIN32__

#include <windows.h>

// UNIX //////////////////////////////////////////
#else

#include <unistd.h>
#define Sleep( msec ) usleep(( msec ) * 1000 )

//////////////////////////////////////////////////
#endif
const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

static bool _check_pressed( uint8_t sn )
{
	int val;

	if ( sn == SENSOR__NONE_ ) {
		return ( ev3_read_keys(( uint8_t *) &val ) && ( val & EV3_KEY_UP ));
	}
	return ( get_sensor_value( 0, sn, &val ) && ( val != 0 ));
}
int get_color(uint8_t sn_color);
void RECOGNIZE_OBST(void);


int main( void )
{

	int i;
	uint8_t sn;
	FLAGS_T state;
	uint8_t sn_touch;
	uint8_t sn_color;
	uint8_t sn_compass;
	uint8_t sn_sonar;
	uint8_t sn_mag;
	char s[ 256 ];
	int val;
	float value;
	uint32_t n, ii;
#ifndef __ARM_ARCH_4T__
	/* Disable auto-detection of the brick (you have to set the correct address below) */
	ev3_brick_addr = "192.168.0.204";

#endif
	if ( ev3_init() == -1 ) return ( 1 );

#ifndef __ARM_ARCH_4T__
	printf( "The EV3 brick auto-detection is DISABLED,\nwaiting %s online with plugged tacho...\n", ev3_brick_addr );


#endif

	ev3_sensor_init();
	if ( ev3_search_sensor( LEGO_EV3_COLOR, &sn_color, 0 )) {
		printf( "COLOR sensor is found, reading COLOR...\n" );
	}
	while (1) {
		val=get_color(sn_color);
	}
}
		//Run all sensors
		//	to be seen
		/*set_sensor_mode(sn_color,"RGB_RAW");*/
		/*set_sensor_mode(sn_color,"COL_COLOR");*/

		void RECOGNIZE_OBST(void) {
			

		}

		int get_color(uint8_t sn_color){

			int val;
			if ( !get_sensor_value( 0, sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
				val = 0;
			}
			printf("val is %d\n",val);
			printf( "\r(%s) \n", color[ val ]);
			fflush( stdout );
			return val;
		}



