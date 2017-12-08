#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_tacho.h"
#include "messages.h"

#define ARM_MOTOR_PORT 65
#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 67
#define MAX_SPEED 1050

#define degree_to_lin 260/90 //to be tuned
#define ANGULAR_SPEED 50


#define Sleep( msec ) usleep(( msec ) * 1000 )


uint8_t arm_motor_sn;

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

void TURN_DEGREES( int ang_speed, double angle )
{
    _run_to_rel_pos( ang_speed, -angle*degree_to_lin, ang_speed, angle*degree_to_lin);
    /*_wait_stopped = 1;*/ //dunno what this refers to so I keep it
}


int movement_init(){
    printf("I started\n");
    ev3_tacho_init();

    int a = ev3_search_tacho_plugged_in(ARM_MOTOR_PORT, 0, &arm_motor_sn, 0 );
    ev3_search_tacho_plugged_in(LEFT_MOTOR_PORT, 0, &motor[0], 0 );
    ev3_search_tacho_plugged_in(RIGHT_MOTOR_PORT, 0, &motor[1], 0 );
    /*printf("Not working %d\n",a);*/
    if (a==0) {
        printf("Not working %d\n",a);
    }

    /* Decide how the motors should behave when stopping.
    We have the alternatives COAST, BRAKE, and HOLD. They result in harder/softer breaking */
	set_tacho_stop_action_inx( motor[0], TACHO_BRAKE );
    set_tacho_stop_action_inx( motor[1], TACHO_BRAKE );
    set_tacho_stop_action_inx( arm_motor_sn, TACHO_BRAKE );

    set_tacho_speed_sp(motor[0], MAX_SPEED * 2 / 3 );
    set_tacho_speed_sp(motor[1], MAX_SPEED * 2 / 3 );
    set_tacho_speed_sp(arm_motor_sn, MAX_SPEED/5);



    return 0;
}

void drop(){
    printf("DROP CALLED\n");
    /*set_tacho_position_sp(arm_motor_sn, 150);*/
    /*set_tacho_command_inx(arm_motor_sn, TACHO_RUN_TO_REL_POS);*/
    /*Sleep(2000);*/
    printf("I am up now I will go down\n");
    set_tacho_position_sp(arm_motor_sn, -150);
    set_tacho_command_inx(arm_motor_sn, TACHO_RUN_TO_REL_POS);
    Sleep(2000);
    printf("I am down so finished\n");
    set_tacho_position_sp(arm_motor_sn, 150);
    set_tacho_command_inx(arm_motor_sn, TACHO_RUN_TO_REL_POS);
    Sleep(2000);
}

void stop(){
    set_tacho_command_inx(motor[0], TACHO_STOP);
    set_tacho_command_inx(motor[1], TACHO_STOP);
}
void forward(){
    set_tacho_command_inx(motor[0], TACHO_RUN_FOREVER);
    set_tacho_command_inx(motor[1], TACHO_RUN_FOREVER);
}

void movement_start() {
    printf("I have been called\n");
    movement_init();

    //testing drop object function
    drop();
    //TESTING THE ARMS
    //from testing a good rel_pos value is between 120 and 150
    /*int i;*/
    /*for (i = 0; i < 1000; i+=10) {*/
        /*printf("This is the rel value : %d\n",i);*/
        /*set_tacho_speed_sp(arm_motor_sn, MAX_SPEED/5);*/
        /*set_tacho_position_sp(arm_motor_sn, i );*/
        /*set_tacho_command_inx(arm_motor_sn, TACHO_RUN_TO_REL_POS);*/
        /*Sleep(1000);*/
        /*set_tacho_position_sp(arm_motor_sn, -i );*/
        /*set_tacho_command_inx(arm_motor_sn, TACHO_RUN_TO_REL_POS);*/
        /*Sleep(1000);*/
    /*}*/
}
