#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#include "ev3.h"
#include "ev3_tacho.h"
#include "messages.h"

#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 67
#define RUN_SPEED 500 // Max is 1050
#define DEGREE_TO_LIN 2.05


#define Sleep( msec ) usleep(( msec ) * 1000 )

#define POS_CALC_PERIOD_MS 10

enum name {L, R};
uint8_t motor[2];

struct coord {
	float x;
	float y;
} coord;

// angle between robot nose and the x axis
float heading = 90;

// true when robot is moving straight
bool do_track_position = false;

void *position_tracker(void* param) {
	int rspeed, lspeed;
	float llin_speed, rlin_speed, lpos, rpos, distance;
	double radius = 2.7;

	while (1) {
		if (do_track_position) {
			get_tacho_speed(motor[L], &lspeed); 		// deg per second
			get_tacho_speed(motor[R], &rspeed); 		// deg per second
			llin_speed = lspeed * radius * M_PI/180;	// cm per second
			rlin_speed = rspeed * radius * M_PI/180;    // cm per second
			lpos = llin_speed * POS_CALC_PERIOD_MS / 1000;
			rpos = rlin_speed * POS_CALC_PERIOD_MS / 1000;
			distance = (lpos+rpos)/2;
			coord.x += distance * cos(heading*M_PI/180);
			coord.y += distance * sin(heading*M_PI/180);
		} else {
			distance = 0;
			lpos = 0;
			rpos = 0;
		}

		printf("%f, %f\n", coord.x, coord.y);

		Sleep(POS_CALC_PERIOD_MS);
	}
}

void *position_sender(void* queues) {
	mqd_t* tmp = (mqd_t*)queues;
	mqd_t movement_queue_to_main = tmp[0];
    double cal_factor = 2.2;

	for(;;) {
		send_message(movement_queue_to_main, MESSAGE_POS_X, coord.x * cal_factor + 0.5);
		send_message(movement_queue_to_main, MESSAGE_POS_Y, coord.y * cal_factor + 0.5);
		Sleep(1000);
	}

}

int movement_init(){
	ev3_tacho_init();

	ev3_search_tacho_plugged_in(LEFT_MOTOR_PORT, 0, &motor[L], 0 );
	ev3_search_tacho_plugged_in(RIGHT_MOTOR_PORT, 0, &motor[R], 0 );

	/* Decide how the motors should behave when stopping.
	We have the alternatives COAST, BRAKE, and HOLD. They result in harder/softer breaking */
	set_tacho_stop_action_inx( motor[L], TACHO_BRAKE );
	set_tacho_stop_action_inx( motor[R], TACHO_BRAKE );

	set_tacho_speed_sp(motor[L], RUN_SPEED );
	set_tacho_speed_sp(motor[R], RUN_SPEED );

	return 0;
}

void stop(){
	do_track_position = false;
	set_tacho_command_inx(motor[L], TACHO_STOP);
	set_tacho_command_inx(motor[R], TACHO_STOP);
}

void forward(){
	do_track_position = true;
    set_tacho_speed_sp(motor[L], RUN_SPEED );
    set_tacho_speed_sp(motor[R], RUN_SPEED );
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

void *movement_start(void* queues) {

	printf("Movement Started\n");
	
	mqd_t* tmp = (mqd_t*)queues;
	mqd_t movement_queue_from_main = tmp[0];
	mqd_t movement_queue_to_main = tmp[1];

	pthread_t position_tracker_thread, position_sender_thread;
	pthread_create(&position_tracker_thread, NULL, position_tracker, NULL);
	pthread_create(&position_sender_thread, NULL, position_sender, (void*)&movement_queue_to_main);

	for(;;) {
		turn_degrees(100, 90);
		Sleep(2000);
		turn_degrees(100, -90);
        Sleep(2000);
	}

	while(1) {
	   
		uint16_t command, value;
		get_message(movement_queue_from_main, &command, &value);
		
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