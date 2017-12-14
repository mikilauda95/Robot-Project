#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_tacho.h"
#include "messages.h"

#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 67
#define MAX_SPEED 1050
#define DEGREE_TO_LIN 2.10

#define Sleep( msec ) usleep(( msec ) * 1000 )


enum name {L, R};
uint8_t motor[2];

int movement_init(){
    ev3_tacho_init();

    ev3_search_tacho_plugged_in(LEFT_MOTOR_PORT, 0, &motor[L], 0 );
    ev3_search_tacho_plugged_in(RIGHT_MOTOR_PORT, 0, &motor[R], 0 );

    /* Decide how the motors should behave when stopping.
    We have the alternatives COAST, BRAKE, and HOLD. They result in harder/softer breaking */
	set_tacho_stop_action_inx( motor[L], TACHO_BRAKE );
    set_tacho_stop_action_inx( motor[R], TACHO_BRAKE );

    set_tacho_speed_sp(motor[L], MAX_SPEED * 2 / 3 );
    set_tacho_speed_sp(motor[R], MAX_SPEED * 2 / 3 );

    return 0;
}

void stop(){
    set_tacho_command_inx(motor[L], TACHO_STOP);
    set_tacho_command_inx(motor[R], TACHO_STOP);
}
void forward(){
    set_tacho_command_inx(motor[L], TACHO_RUN_FOREVER);
    set_tacho_command_inx(motor[R], TACHO_RUN_FOREVER);
}


void turn_degrees(int ang_speed, double angle) {
    set_tacho_speed_sp( motor[L], ang_speed );
	set_tacho_speed_sp( motor[R], ang_speed );
	set_tacho_position_sp( motor[L], angle * DEGREE_TO_LIN );
	set_tacho_position_sp( motor[R], -(angle * DEGREE_TO_LIN) );
	multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
    int spd;
    get_tacho_speed(motor[L], &spd);
    while ( spd != 0 ) { // Block until done turning
        get_tacho_speed(motor[L], &spd);
        Sleep(1);
    }
}

void *movement_start() {

    mqd_t movement_queue_to_main = init_queue("/movement_to_main", O_CREAT | O_WRONLY);
    mqd_t movement_queue_from_main = init_queue("/movement_from_main", O_CREAT | O_RDONLY);
    printf("Movement Started\n");
    forward();
    Sleep(1000);
    stop();
    while(1) {
       
        uint16_t command, value;
        int bytes = get_message(movement_queue_from_main, &command, &value);
        // Message queue is non blocking. Read until we get a value
        while(bytes == -1) {
            bytes = get_message(movement_queue_from_main, &command, &value);

            Sleep(1);
        }
        printf("Got message %d with value %d \n", command, value);
        
        if (command == MESSAGE_TURN) {
            stop();
            set_tacho_command_inx(motor[L], TACHO_RUN_FOREVER);
            Sleep(100);
            stop();
        } else if (command == MESSAGE_FORWARD) {
            forward();
        }
        
        Sleep(10);
    }
}
