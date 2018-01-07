#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_sensor.h"
#include "messages.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )

void *sensors_start(void *queues){
    uint8_t sonar_sn;
    uint8_t gyro_sn;

    float sonar_value;
    float gyro_value;
    
    ev3_sensor_init();
    ev3_search_sensor(LEGO_EV3_US, &sonar_sn, 0);
    ev3_search_sensor(LEGO_EV3_GYRO, &gyro_sn, 0);
    // Reset the gyro at startup
    set_sensor_mode_inx(gyro_sn, GYRO_GYRO_RATE);
    set_sensor_mode_inx(gyro_sn, GYRO_GYRO_ANG);

    mqd_t* tmp = (mqd_t*)queues;
	mqd_t queue_sensor_to_main = tmp[0];

    while(1){
        get_sensor_value0(sonar_sn, &sonar_value);
        get_sensor_value0(gyro_sn, &gyro_value);
        int gyro = (int)gyro_value%360;
        if (gyro < 0) {
            gyro +=360;
        }
        gyro = 360 - gyro; // flip the gyro so that it increases anti clockwise. 
        send_message(queue_sensor_to_main, MESSAGE_SONAR, (int16_t)(sonar_value + 0.5));
        send_message(queue_sensor_to_main, MESSAGE_ANGLE, (int16_t)(gyro + 0.5));
        Sleep(40);
    }
}