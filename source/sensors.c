#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_sensor.h"
#include "messages.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )
/*const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };*/
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

uint8_t color_sn;
float color_value;
int val;

void sensors_start(){

	ev3_sensor_init();
	if ( !ev3_search_sensor( LEGO_EV3_COLOR, &color_sn, 0 )) {
		printf( "error: color sensor not found\n" );
	}
	else{
		printf("found it \n");
	}
}

void color_read(){
/*char* color_read(){*/
	/*if ( !get_sensor_value(0 , color_sn, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {*/
		/*val = 0;*/
	/*}*/

	/*printf("this is the return of get_sensor_value0 : %d\n", val);*/
	/*val=(int) color_value;*/
	/*printf( "\r(%s) \n", color[val]);*/
	/*fflush( stdout );*/
	/*return color[val] ;*/
	/*return 1;*/
}

