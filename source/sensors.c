#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_sensor.h"
#include "messages.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )
const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

void sensors_start(){
    uint8_t sonar_sn;
    uint8_t compass_sn;
    uint8_t color_sn;
    float compass_value;
    float sonar_value;
    float color_value;

    ev3_sensor_init();
    ev3_search_sensor( LEGO_EV3_US, &sonar_sn, 0 );
    ev3_search_sensor( HT_NXT_COMPASS, &compass_sn, 0 );
    if ( !ev3_search_sensor( LEGO_EV3_COLOR, &color_sn, 0 )) {
        printf( "error: color sensor not found\n" );
    }

   char * color_read{
        if ( !get_sensor_value( 0, sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
            val = 0;
        }
        printf( "\r(%s) \n", color[ val ]);
        fflush( stdout );
        return color[val] ;
}
   // while(1){
     //   get_sensor_value0(compass_sn, &compass_value );
       // get_sensor_value0(sonar_sn, &sonar_value );
        get_sensor_value0(color_sn, &color_value );
        //send to main
       // send_message(sensors_queue, MESSAGE_SONAR, (uint16_t)sonar_value);
       // Sleep(100);
   // }
}