#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_tacho.h"
#include "messages.h"

#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 67
#define MAX_SPEED 1050

#define Sleep( msec ) usleep(( msec ) * 1000 )


uint8_t left_motor_sn;
uint8_t right_motor_sn;

int movement_init(){
    ev3_init();
    ev3_tacho_init();

    ev3_search_tacho_plugged_in(LEFT_MOTOR_PORT, 0, &left_motor_sn, 0 );
    ev3_search_tacho_plugged_in(RIGHT_MOTOR_PORT, 0, &right_motor_sn, 0 );

    /* Decide how the motors should behave when stopping.
    We have the alternatives COAST, BRAKE, and HOLD. They result in harder/softer breaking */
	set_tacho_stop_action_inx( left_motor_sn, TACHO_BRAKE );
    set_tacho_stop_action_inx( right_motor_sn, TACHO_BRAKE );

    set_tacho_speed_sp(left_motor_sn, MAX_SPEED * 2 / 3 );
    set_tacho_speed_sp(right_motor_sn, MAX_SPEED * 2 / 3 );

    return 0;
}

void stop() {
    set_tacho_command_inx(left_motor_sn, TACHO_STOP);
    set_tacho_command_inx(right_motor_sn, TACHO_STOP);
}

void forward(int speed) {
    set_tacho_speed_sp(left_motor_sn, speed );
    set_tacho_speed_sp(right_motor_sn, speed );
    set_tacho_command_inx(left_motor_sn, TACHO_RUN_FOREVER);
    set_tacho_command_inx(right_motor_sn, TACHO_RUN_FOREVER);
}

void turn_degrees(int ang_speed, double angle) {
     _run_to_rel_pos( ang_speed, -angle*degree_to_lin, ang_speed, angle*degree_to_lin);
     // Block until the motor is done turning
     int temp;
     while (get_tacho_speed_sp(left_motor_sn, *temp) != 0) {
        Sleep(1);
     }
}

void *movement_start() {

    movement_init();
    mqd_t movement_queue = init_queue("/movement", O_CREAT | O_RDWR);

    while(1) {
        uint16_t command, value;
        get_message(movement_queue, &command, &value);
        switch (command) {
            case MSG_MOV_TURN:
                stop();
                turn_degrees(20, (double)value);
                send_message(movement_queue, MSG_MOV_TURN_COMPLETE, 0);
                break;
            case MSG_MOV_RUN_FORWARD:
                forward(MAX_SPEED * 2/3);
                break;
        }
        Sleep(10);
    }
}
