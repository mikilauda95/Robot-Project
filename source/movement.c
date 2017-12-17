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
#define ANG_SPEED 75 // Wheel speed when turning
#define DEGREE_TO_LIN 2.05
#define COUNT_PER_ROT 360 // result of get_tacho_count_per_rot
#define WHEEL_RADIUS 2.7

#define Sleep( msec ) usleep(( msec ) * 1000 )

#define POS_CALC_PERIOD_MS 10

enum name {L, R};
uint8_t motor[2];

struct coord {
	float x;
	float y;
} coord;

// angle between robot nose and the x axis
int heading = 90;

// true when robot is moving straight
bool do_track_position = false;
int prev_l_pos = 0;
int prev_r_pos = 0;

void update_position() {
	// this function relies on the position to be set to zero after every turn.
	// the idea is to call this every time we whish to send our position. 
	int rpos, lpos;
	get_tacho_position(motor[R], &rpos);
	get_tacho_position(motor[L], &lpos);
	float distance = (((lpos-prev_l_pos)+(rpos-prev_r_pos))/2);
	distance = (distance/COUNT_PER_ROT) * 2*M_PI*WHEEL_RADIUS;
	prev_l_pos = lpos;
	prev_r_pos = rpos;
	coord.x += distance * cos( heading*M_PI/180);
	coord.y += distance * sin( heading*M_PI/180); 
}

void *position_sender(void* queues) {
	mqd_t* tmp = (mqd_t*)queues;
	mqd_t movement_queue_to_main = tmp[0];

	for(;;) {
		if(do_track_position){
			update_position();
		}
		uint16_t x = (int16_t) (coord.x + 0.5);
		uint16_t y = (int16_t) (coord.y + 0.5);
		printf("position_sender: Sending x: %d, y: %d\n",x,y);
		send_message(movement_queue_to_main, MESSAGE_POS_X, x);
		send_message(movement_queue_to_main, MESSAGE_POS_Y, y);
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

	coord.x = 40.0;
	coord.y = 10.0;

	return 0;
}

void stop(){
	update_position(); // make sure to update the position when we stop. 
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

void turn_degrees_gyro(float delta, int angle_speed, mqd_t sensor_queue) {

	uint16_t command;
	int16_t current_angle;
	get_message(sensor_queue, &command, &current_angle);
	
	float target = current_angle + delta;
	printf("turn deg: delta = %f, current = %d, target = %f\n", delta, current_angle, target);

	if (delta > 0) {
		set_tacho_speed_sp( motor[L], angle_speed );
		set_tacho_speed_sp( motor[R], -angle_speed );
		multi_set_tacho_command_inx(motor, TACHO_RUN_FOREVER);
	}
	else {
		set_tacho_speed_sp( motor[L], -angle_speed );
		set_tacho_speed_sp( motor[R], angle_speed );
		multi_set_tacho_command_inx(motor, TACHO_RUN_FOREVER);
	}

	for (;;) {
		get_message(sensor_queue, &command, &current_angle);
		
		if (delta < 0) {
			if (current_angle < target) {
				set_tacho_command_inx(motor[L], TACHO_STOP);
				set_tacho_command_inx(motor[R], TACHO_STOP);
				break;
			}
		}else {
			if (current_angle > (target - 5) ){
				set_tacho_command_inx(motor[L], TACHO_STOP);
				set_tacho_command_inx(motor[R], TACHO_STOP);
				break;
			}
		}

	}
	
}


void *movement_start(void* queues) {

	printf("Movement Started\n");
	
	mqd_t* tmp = (mqd_t*)queues;
	mqd_t movement_queue_from_main = tmp[0];
	mqd_t movement_queue_to_main = tmp[1];

	set_tacho_position(motor[L], 0);
	set_tacho_position(motor[R], 0);

	pthread_t position_tracker_thread, position_sender_thread;
	//pthread_create(&position_tracker_thread, NULL, position_tracker, NULL);
	pthread_create(&position_sender_thread, NULL, position_sender, (void*)&movement_queue_to_main);

	while(1) {
	   
		uint16_t command;
		int16_t value;
		get_message(movement_queue_from_main, &command, &value);

		switch (command) {
			case MESSAGE_TURN_DEGREES:
				stop();
				Sleep(500);
				turn_degrees_gyro(value, ANG_SPEED, movement_queue_from_main);
				printf("Heading was %d\r\n", heading);
				heading = (heading + value) % 360;
				printf("Heading is now %d\r\n", heading);
				send_message(movement_queue_to_main, MESSAGE_TURN_COMPLETE, 0);
				// set position to 0 after a turn. It's important that motors are not turning when this is done
				set_tacho_position(motor[L], 0);
				set_tacho_position(motor[R], 0);
				prev_l_pos = 0;
				prev_r_pos = 0;
			break;
			
			case MESSAGE_FORWARD:
				forward();
			break;
			
			case MESSAGE_STOP:
				stop();
			break;
		}
	}
}