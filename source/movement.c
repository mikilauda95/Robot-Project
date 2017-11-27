#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//personal libraries
#include "constants.h"
#include "messages.h"
//ev3 libraries
#include "ev3.h"
#include "coroutine.h"
#include "ev3_port.h"
#include "ev3_sensor.h"
#include "ev3_tacho.h"

#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 67
#define MAX_SPEED 1050
#define degree_to_lin 260/90

#define Sleep( msec ) usleep(( msec ) * 1000 )



/*uint8_t left_motor_sn;*/
/*uint8_t right_motor_sn;*/


enum name {L, R};
uint8_t motor[2];


static void _run_to_rel_pos( int l_speed, int l_pos, int r_speed, int r_pos )
{
	set_tacho_speed_sp( motor[L], l_speed );
	set_tacho_speed_sp( motor[R], r_speed );
	set_tacho_position_sp( motor[L], l_pos );
	set_tacho_position_sp( motor[R], r_pos );
	multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}


/*void turn_degrees(char *direction, double degrees, double angular_speed){*/
    /*int lin_speed=(int) (angular_speed*LIN_TO_ANG);//to review*/
    /*int lin_pos= (int) degrees*LIN_TO_ANG;//to review*/
    /*if (strcmp(direction, "LEFT")) {*/
        /*_run_to_rel_pos(0, 0, lin_speed, lin_pos);*/
    /*}*/
    /*else {*/
        /*_run_to_rel_pos( lin_speed, lin_pos, 0, 0);*/
    /*}*/
/*}*/

void TURN_DEGREES( int ang_speed, double angle )
{
    _run_to_rel_pos( ang_speed, -angle*degree_to_lin, ang_speed, angle*degree_to_lin);
    /*_wait_stopped = 1;*/ //dunno what this refers to so I keep it
}



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

void movement_start() {
    movement_init();

    // Testing. Run forward for one second.
    set_tacho_command_inx(motor[L], TACHO_RUN_FOREVER);
    set_tacho_command_inx(motor[R], TACHO_RUN_FOREVER);
    Sleep(1000);
    stop();

    mqd_t movement_queue = init_queue("/movement", O_CREAT | O_RDONLY);

    while(1) {
        uint16_t command, value;
        get_message(movement_queue, &command, &value);
        //normal mode
        if (command == MESSAGE_TURN) {
            stop();
            set_tacho_command_inx(motor[L], TACHO_RUN_FOREVER);
        } else if (command == MESSAGE_FORWARD) {
            forward();
        }
        else if (command== MESSAGE_SCANN) {
            //implement the scanning function for the movement
            double curr_pos;
            double curr_ang;
            //implement two receives for getting the curr position and the current angle
            //
            int step_degrees=(int)( 360/MAX_SCAN_NUM );
            for (int i = 0; i < MAX_SCAN_NUM; ++i) {

                //need to set right values
                TURN_DEGREES(20, step_degrees); //or right
                //will it wait till it has completed the movement? or should I sleep?
                send_message(movement_queue, MESSAGE_AVGCOMPASS, 0);
                get_message(movement_queue, &command, &value);
                if (command != MESSAGE_CONTINUE) {
                    fprintf(stderr, "ERROR DURING THE SCAN\n");
                    exit(1);
                }
            }
        }

        Sleep(10);
    }
}

