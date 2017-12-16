#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_sensor.h"
#include "messages.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )


void *sensors_start(void *queues){
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

    mqd_t* tmp = (mqd_t*)queues;
	mqd_t queue_sensor_to_main = tmp[0];

    while(1){
        get_sensor_value0(sonar_sn, &sonar_value );
        get_sensor_value0(gyro_sn, &gyro_value );

        //send to main
        send_message(queue_sensor_to_main, MESSAGE_SONAR, (int16_t)(sonar_value + 0.5));
        send_message(queue_sensor_to_main, MESSAGE_GYRO, (int16_t)(gyro_value + 0.5));
        Sleep(100);
    }
}