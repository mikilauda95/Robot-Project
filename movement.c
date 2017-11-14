#include <stdio.h>
#include <stdlib.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"
#include "messages.h"

#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 67
#define MAX_SPEED 1050

#define Sleep( msec ) usleep(( msec ) * 1000 )


int movement_init(){
    int n_motors = ev3_tacho_init();
    int n_sensors = ev3_sensor_init();
    if (n_motors < 0 || n_sensors < 0 ) {
        return -1;
    } else {
        return n_motors + n_sensors;
    }
}

uint8_t left_motor_sn;
uint8_t right_motor_sn;

void stop(){
    set_tacho_command_inx(left_motor_sn, TACHO_STOP);
    set_tacho_command_inx(right_motor_sn, TACHO_STOP);
}
void set_dir(int deg){

}

void set_speed(int speed){
    
}

void movement_start() {
    movement_init();

    ev3_search_tacho_plugged_in(LEFT_MOTOR_PORT, 0, &left_motor_sn, 0 );
    ev3_search_tacho_plugged_in(RIGHT_MOTOR_PORT, 0, &right_motor_sn, 0 );

    /* Decide how the motors should behave when stopping.
    We have the alternatives COAST, BRAKE, and HOLD. They result in harder/softer breaking */
	set_tacho_stop_action_inx( left_motor_sn, TACHO_BRAKE );
    set_tacho_stop_action_inx( right_motor_sn, TACHO_BRAKE );

    set_tacho_speed_sp(left_motor_sn, MAX_SPEED * 2 / 3 );
    set_tacho_speed_sp(right_motor_sn, MAX_SPEED * 2 / 3 );

    set_tacho_command_inx(left_motor_sn, TACHO_RUN_FOREVER);
    set_tacho_command_inx(right_motor_sn, TACHO_RUN_FOREVER);
    Sleep(1000);
    stop();

    while(1) {
        int message = get_integer_from_mq();
        printf("got message %d \n", message);
        Sleep(100);
        // check messages
        // do what main tells me
        // repeat
    }
}
