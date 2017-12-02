#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_sensor.h"
#include "messages.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )


void sensors_start(){
    uint8_t sonar_sn;
    uint8_t compass_sn;
    float compass_value;
    float sonar_value;

    ev3_sensor_init();
    ev3_search_sensor( LEGO_EV3_US, &sonar_sn, 0 );
    ev3_search_sensor( HT_NXT_COMPASS, &compass_sn, 0 );

    mqd_t sensors_queue = init_queue("/sensors", O_CREAT | O_WRONLY);
    uint16_t command, value;

    while(1){
        //code implemented to manage the compass values when requested
        get_message(sensors_queue, &command, &value);
        switch (command) {
            case MESSAGE_AVGCOMPASS://this is not really the average function, but put this just to try
                get_sensor_value0(compass_sn, &compass_value);
                send_message(sensors_queue, MESSAGE_AVGCOMPASS, (uint16_t)compass_value);
                break;
            default:
                //from here there is the standard code for the first example, the only difference is that you should send a message to activate because it is blocked by the get_message function above
                get_sensor_value0(compass_sn, &compass_value );
                get_sensor_value0(sonar_sn, &sonar_value );
                //send to main
                send_message(sensors_queue, MESSAGE_SONAR, (uint16_t)sonar_value);
                Sleep(100);
                break;
        }
    }
}
