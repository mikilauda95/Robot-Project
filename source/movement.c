#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_tacho.h"
#include "messages.h"
#include <pthread.h>

#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 67
#define MAX_SPEED 1050

#define Sleep( msec ) usleep(( msec ) * 1000 )


uint8_t left_motor_sn;
uint8_t right_motor_sn;
pthread_t movid;

int movement_init(){
    ev3_tacho_init();

    ev3_search_tacho_plugged_in(LEFT_MOTOR_PORT, 0, &left_motor_sn, 0 );
    ev3_search_tacho_plugged_in(RIGHT_MOTOR_PORT, 0, &right_motor_sn, 0 );

    /* Decide how the motors should behave when stopping.
    We have the alternatives COAST, BRAKE, and HOLD. They result in harder/softer breaking */
	//speed in cm per secondset_tacho_stop_action_inx( left_motor_sn, TACHO_BRAKE );
    set_tacho_stop_action_inx( right_motor_sn, TACHO_BRAKE );

    set_tacho_speed_sp(left_motor_sn, MAX_SPEED  / 5 );
    set_tacho_speed_sp(right_motor_sn, MAX_SPEED / 5 );

    return 0;
}

void* tracepos()
{
    int rdistance, ldistance, rspeed,lspeed;
	double R;
	double llin_speed, rlin_speed;
	double rpos, lpos=0;
	R=2.7;//5,4 of diameter//centimeter
	double pi=3.14;
    while (1) {
        /*get_tacho_position_sp(left_motor_sn, &ldistance);*/
        /*printf("The left distance is %d\n",ldistance);*/
        /*get_tacho_position_sp(right_motor_sn, &rdistance);*/
        /*printf("The right distance is %d\n",rdistance);*/
        get_tacho_speed(left_motor_sn, &lspeed);//tacho speed gives the degree per second
        printf("The left speed is %d\n",lspeed);
        get_tacho_speed(right_motor_sn, &rspeed);
        printf("The right speed is %d\n",rspeed);
        /*get_tacho_count_per_rot(left_motor_sn, &lspeed);*/
        /*printf("The left deg/s is %d\n",lspeed);*/
        /*get_tacho_count_per_rot(right_motor_sn, &rspeed);*/
        /*printf("The right deg/s is %d\n",rspeed);*/
		llin_speed=lspeed*R*pi/180;//speed in cm per second
        printf("The left linear speed is %f\n",llin_speed);
		rlin_speed=rspeed*R*pi/180;//speed in cm per second

        printf("The right linear speed is %f\n",rlin_speed);
        Sleep(100);
		lpos+=llin_speed*0.1;
		printf("lpos is %f\n",lpos);
		rpos+=rlin_speed*0.1;
		printf("rpos is %f\n",rpos);
    }
}

void stop(){
    set_tacho_command_inx(left_motor_sn, TACHO_STOP);
    set_tacho_command_inx(right_motor_sn, TACHO_STOP);
}
void forward(){
    set_tacho_command_inx(left_motor_sn, TACHO_RUN_FOREVER);
    set_tacho_command_inx(right_motor_sn, TACHO_RUN_FOREVER);
}

void movement_start() {
    movement_init();

    // Testing. Run forward for one second.
    /*set_tacho_command_inx(left_motor_sn, TACHO_RUN_FOREVER);*/
    /*set_tacho_command_inx(right_motor_sn, TACHO_RUN_FOREVER);*/
    /*Sleep(1000);*/
    stop();
    if(pthread_create(&movid,NULL, &tracepos, NULL)!=0){
        printf("Failed the creation of the thread\n");
    }
    Sleep(1000);


    mqd_t movement_queue = init_queue("/movement", O_CREAT | O_RDONLY);
    for (int i = 0; i < 1; ++i) {
        forward();
        Sleep(5000);
        stop();
        Sleep(2000);
    }

    /*while(1) {*/
        /*uint16_t command, value;*/
        /*get_message(movement_queue, &command, &value);*/
        /*if (command == MESSAGE_TURN) {*/
            /*stop();*/
            /*set_tacho_command_inx(left_motor_sn, TACHO_RUN_FOREVER);*/
        /*} else if (command == MESSAGE_FORWARD) {*/
            /*forward();*/
        /*}*/
        /*Sleep(10);*/
    /*}*/
}
