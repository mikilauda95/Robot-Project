#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_sensor.h"
#include "messages.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )


void *sensors_start(){
    uint8_t sonar_sn;
    uint8_t compass_sn;
    uint8_t gyro_sn;

    float compass_value;
    float sonar_value;
    float gyro_value;

    ev3_sensor_init();
    ev3_search_sensor(LEGO_EV3_US, &sonar_sn, 0);
    ev3_search_sensor(HT_NXT_COMPASS, &compass_sn, 0);
     ev3_search_sensor(LEGO_EV3_GYRO, &gyro_sn, 0);

    mqd_t sensors_queue = init_queue("/sensors", O_CREAT | O_WRONLY);

    while(1){
        get_sensor_value0(compass_sn, &compass_value );
        get_sensor_value0(sonar_sn, &sonar_value );
        get_sensor_value0(gyro_sn, &gyro_value );
        //send to main
        send_message(sensors_queue, MESSAGE_SONAR, (uint16_t)sonar_value);
        send_message(sensors_queue, MESSAGE_GYRO, (uint16_t)gyro_value);
        Sleep(100);
    }
}