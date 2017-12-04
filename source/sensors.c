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
    uint8_t sn_color;
    float compass_value;
    float sonar_value;
    float color_val;

    ev3_sensor_init();
    ev3_search_sensor( LEGO_EV3_US, &sonar_sn, 0 );
    ev3_search_sensor( LEGO_EV3_COLOR, &sn_color, 0 );
    ev3_search_sensor( HT_NXT_COMPASS, &compass_sn, 0 );

    mqd_t sensors_queue = init_queue("/sensors", O_CREAT | O_RDWR);
    uint16_t command, value;
    printf("sensors have been started\n");
    /*while (1) {*/

        /*get_message(sensors_queue, &command, &value);*/
        /*[>get_sensor_value0(compass_sn, &compass_value );<]*/
        /*[>printf("measured the compass: %d\n");<]*/
        /*get_sensor_value0(sonar_sn, &sonar_value );*/
        /*printf("measured the US: %d\n", sonar_value);*/
        /*//send to main*/
        /*send_message(sensors_queue, MESSAGE_SONAR, (uint16_t)sonar_value);*/
        /*Sleep(10);*/
    /*}*/

    /*while(1){*/
        //code implemented to manage the compass values when requested
        get_message(sensors_queue, &command, &value);
        printf("Received this command from the main : %d\n",command);
        switch (command) {
            case MESSAGE_AVGCOMPASS://this is not really the average function, but put this just to try
                get_sensor_value0(compass_sn, &compass_value);
                send_message(sensors_queue, MESSAGE_AVGCOMPASS, (uint16_t)compass_value);
                break;
            case MESSAGE_COLORSENS:
                while (1) {
                    if ( !get_sensor_value0(sn_color, &color_val ) || ( color_val < 0 ) || ( color_val >= COLOR_COUNT )) {
                        color_val = 0;
                    }
                    printf( "\r(%s)", color[(int)color_val]);
                    /*send_message(sensors_queue, MESSAGE_COLORSENS, (uint16_t)color_val);*/
                    fflush( stdout );
                }
            case MESSAGE_START:
                printf("Received the START FOR SENSORS\n");
                while (1) {
                    /*get_sensor_value0(compass_sn, &compass_value );*/
                    /*printf("measured the compass: %d\n");*/
                    get_sensor_value0(sonar_sn, &sonar_value );
                    printf("measured the US: %d\n", sonar_value);
                    //send to main
                    send_message(sensors_queue, MESSAGE_SONAR, (uint16_t)sonar_value);
                    Sleep(10);
                }
                break;
                //from here there is the standard code for the first example, the only difference is that you should send a message to activate because it is blocked by the get_message function above
            default:
                break;
        }
    }
/*}*/
