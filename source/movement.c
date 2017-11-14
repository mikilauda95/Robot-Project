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

// WIN32
#ifdef __WIN32__
#include <windows.h>
// UNIX 
#else
#include <unistd.h>
#define Sleep( msec ) usleep(( msec ) * 1000 )
#endif

uint8_t left_motor_sn;
uint8_t right_motor_sn;

int movement_init(){
    if (ev3_tacho_init() == -1 ) {
        return -1;
    } 
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

void stop(){
    set_tacho_command_inx(left_motor_sn, TACHO_STOP);
    set_tacho_command_inx(right_motor_sn, TACHO_STOP);
}

void movement_start() {
    movement_init();

    // Testing. Run forward for one second.
    set_tacho_command_inx(left_motor_sn, TACHO_RUN_FOREVER);
    set_tacho_command_inx(right_motor_sn, TACHO_RUN_FOREVER);
    Sleep(1000);
    stop();

    mqd_t movement_queue = init_queue("/movement", O_CREAT | O_RDONLY);

    while(1) {
        int message = get_integer_from_mq(movement_queue);

        printf("got message %d with value %d \n", message >> 11 , message & 0x7ff);
        Sleep(100);
        // check messages
        // do what main tells me
        // repeat
    }
}
