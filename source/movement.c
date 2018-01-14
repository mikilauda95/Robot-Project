#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#include "movement.h"
#include "ev3.h"
#include "ev3_tacho.h"
#include "messages.h"
#include "tuning.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )

enum name {L, R};
uint8_t motor[2];
uint8_t sweep_motor;
uint8_t arm_motor;

mqd_t movement_queue_from_main, movement_queue_to_main;

struct coord {
	float x;
	float y;
} coord;

int target_dist;
float current_dist;
// angle between robot nose and the x axis
int heading = START_HEADING;

// true when robot is moving straight
bool do_track_position = false;
bool do_sweep_sonar = false;

int prev_l_pos = 0;
int prev_r_pos = 0;

void wait_for_motor(uint8_t motor) {
	int spd = 0;
	// First we wait until the motor starts spinning
	while (spd  == 0) {
		Sleep(10);
		get_tacho_speed(motor, &spd);
	}
	// Then we block until done turning	
	while ( spd != 0 ) { 
		get_tacho_speed(motor, &spd);
		Sleep(10);
	}
}

void update_position() {
	// this function relies on the position to be set to zero after every turn.
	// the idea is to call this every time we whish to send our position. 
	int rpos, lpos;
	get_tacho_position(motor[R], &rpos);
	get_tacho_position(motor[L], &lpos);
	float distance = (((lpos-prev_l_pos)+(rpos-prev_r_pos))/2);
	distance = (distance/COUNT_PER_ROT) * 2*M_PI*WHEEL_RADIUS;
	current_dist += distance;
	prev_l_pos = lpos;
	prev_r_pos = rpos;
	coord.x += distance * cos(heading*M_PI/180);
	coord.y += distance * sin(heading*M_PI/180);
	// If we reached target within 5cm margin
	if (target_dist > current_dist - 50 && target_dist < current_dist + 50) {
		send_message(movement_queue_to_main, MESSAGE_REACHED_DEST, 0);
	}
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
		
		send_message(movement_queue_to_main, MESSAGE_POS_X, x);
		send_message(movement_queue_to_main, MESSAGE_POS_Y, y);
		Sleep(100);
	}

}


void *sonar_sweeper(void* what) {

	set_tacho_position(sweep_motor, 0);

	int sweep_time = 400;
	int sweep_amplitude = 55;
	bool is_zeroed = true;

	for (;;) {

		if (do_sweep_sonar) {
			set_tacho_position_sp( sweep_motor, sweep_amplitude);
			set_tacho_command_inx(sweep_motor, TACHO_RUN_TO_ABS_POS);
			sweep_amplitude *= -1;
			Sleep(sweep_time);
			is_zeroed = false;
		} else {
			if (!is_zeroed) {
				set_tacho_position_sp( sweep_motor, 0);
				set_tacho_command_inx(sweep_motor, TACHO_RUN_TO_ABS_POS);
				is_zeroed = true;
			}
			Sleep(500);
		}

	}

}

int movement_init(){
	srand(time(NULL));
	ev3_tacho_init();

	ev3_search_tacho_plugged_in(LEFT_MOTOR_PORT, 0, &motor[L], 0 );
	ev3_search_tacho_plugged_in(RIGHT_MOTOR_PORT, 0, &motor[R], 0 );
	ev3_search_tacho_plugged_in(SWEEP_MOTOR_PORT, 0, &sweep_motor, 0 );
	ev3_search_tacho_plugged_in(ARM_MOTOR_PORT, 0, &arm_motor, 0 );

	/* Decide how the motors should behave when stopping.
	We have the alternatives COAST, BRAKE, and HOLD. They result in harder/softer breaking */
	set_tacho_stop_action_inx( motor[L], TACHO_BRAKE );
	set_tacho_stop_action_inx( motor[R], TACHO_BRAKE );
	set_tacho_stop_action_inx( sweep_motor, TACHO_BRAKE );
	set_tacho_stop_action_inx( arm_motor, TACHO_BRAKE );

	set_tacho_speed_sp(motor[L], FORWARD_SPEED );
	set_tacho_speed_sp(motor[R], FORWARD_SPEED );
	set_tacho_speed_sp(arm_motor, FORWARD_SPEED );
	set_tacho_speed_sp(sweep_motor, SWEEP_SPEED );
	
	coord.x = ROBOT_START_X;
	coord.y = ROBOT_START_Y;

	return 0;
}

void stop(){
	update_position(); // make sure to update the position when we stop. 
	do_track_position = false;
	do_sweep_sonar = false;
	set_tacho_command_inx(motor[L], TACHO_STOP);
	set_tacho_command_inx(motor[R], TACHO_STOP);
}

void forward(){
	do_track_position = true;
	do_sweep_sonar = true;
    set_tacho_speed_sp(motor[L], FORWARD_SPEED );
    set_tacho_speed_sp(motor[R], FORWARD_SPEED );
	set_tacho_command_inx(motor[L], TACHO_RUN_FOREVER);
	set_tacho_command_inx(motor[R], TACHO_RUN_FOREVER);
}
void forward2(int distance){
	do_track_position = true;
	do_sweep_sonar = true;
	int tics = (distance * COUNT_PER_ROT)/(2*M_PI * WHEEL_RADIUS);
    set_tacho_speed_sp(motor[L], FORWARD_SPEED );
    set_tacho_speed_sp(motor[R], FORWARD_SPEED );
	set_tacho_position_sp(motor[L], tics);
	set_tacho_position_sp(motor[R], tics);
	set_tacho_command_inx(motor[L], TACHO_RUN_TO_REL_POS);
	set_tacho_command_inx(motor[R], TACHO_RUN_TO_REL_POS);
}


	

void turn_degrees(float angle, int speed) {
	// Turn more slowly if the angle in small
	int turn_speed = (angle>10||angle<-10)?speed:100;
	set_tacho_speed_sp( motor[L], turn_speed );
	set_tacho_speed_sp( motor[R], turn_speed );
	set_tacho_position_sp( motor[L], -angle * DEGREE_TO_LIN );
	set_tacho_position_sp( motor[R], angle * DEGREE_TO_LIN );
	/*multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );*/
    set_tacho_command_inx(motor[R], TACHO_RUN_TO_REL_POS);
    set_tacho_command_inx(motor[L], TACHO_RUN_TO_REL_POS);
	wait_for_motor(motor[L]);

}

void turn_degrees_gyro(float delta, int angle_speed, mqd_t sensor_queue) {

	uint16_t command;
	int16_t current_angle;
	get_message(sensor_queue, &command, &current_angle);
	
	float target = current_angle + delta;

	if (delta > 0) {
		set_tacho_speed_sp( motor[L], angle_speed );
		set_tacho_speed_sp( motor[R], -angle_speed );
		/*multi_set_tacho_command_inx(motor, TACHO_RUN_FOREVER);*/
        set_tacho_command_inx(motor[R], TACHO_RUN_FOREVER);
        set_tacho_command_inx(motor[L], TACHO_RUN_FOREVER);
	}
	else {
		set_tacho_speed_sp( motor[L], -angle_speed );
		set_tacho_speed_sp( motor[R], angle_speed );
		/*multi_set_tacho_command_inx(motor, TACHO_RUN_FOREVER);*/
        set_tacho_command_inx(motor[R], TACHO_RUN_FOREVER);
        set_tacho_command_inx(motor[L], TACHO_RUN_FOREVER);
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

void drop_object()
{
    set_tacho_position_sp(arm_motor, -90);
    set_tacho_command_inx(arm_motor, TACHO_RUN_TO_REL_POS);
    
	wait_for_motor(arm_motor);
	set_tacho_position_sp(arm_motor, 90);
	set_tacho_command_inx(arm_motor, TACHO_RUN_TO_REL_POS);
	wait_for_motor(arm_motor);
	int16_t obj_x = coord.x + OBJECT_OFFSET * cos((heading + 180)/180.0 * M_PI);
	int16_t obj_y = coord.y + OBJECT_OFFSET * sin((heading + 180)/180.0 * M_PI);

	send_message(movement_queue_to_main, MESSAGE_DROP_X, obj_x);
	send_message(movement_queue_to_main, MESSAGE_DROP_Y, obj_y);
	send_message(movement_queue_to_main, MESSAGE_DROP_COMPLETE, 0);
	
}

void *movement_start(void* queues) {

	printf("Movement Started\n");
	
	mqd_t* tmp = (mqd_t*)queues;
	movement_queue_from_main = tmp[0];
	movement_queue_to_main = tmp[1];

	set_tacho_position(motor[L], 0);
	set_tacho_position(motor[R], 0);

	pthread_t position_sender_thread;
	pthread_t sonar_sweeper_thread;
	int scan_dir = -1;
	pthread_create(&position_sender_thread, NULL, position_sender, (void*)&movement_queue_to_main);
	pthread_create(&sonar_sweeper_thread, NULL, sonar_sweeper, NULL);
	while(1) {
		uint16_t command;
		int16_t value;
		get_message(movement_queue_from_main, &command, &value);
		switch (command) {
			case MESSAGE_TURN_DEGREES:
				stop();
				Sleep(150);
				turn_degrees(value, TURN_SPEED);
				send_message(movement_queue_to_main, MESSAGE_TURN_COMPLETE, 0);
				// set position to 0 after a turn. It's important that motors are not turning when this is done
				set_tacho_position(motor[L], 0);
				set_tacho_position(motor[R], 0);
				prev_l_pos = 0;
				prev_r_pos = 0;
			break;
			
			case MESSAGE_FORWARD:
				forward2(target_dist);
			break;
			case MESSAGE_TARGET_DISTANCE:
				target_dist = value;
			break;
			
			case MESSAGE_STOP:
				stop();
				// Forget old target when stopping
				target_dist = -10;
				current_dist = 0;
			break;

			case MESSAGE_SCAN:
				stop();
				Sleep(500);			
				send_message(movement_queue_to_main, MESSAGE_SCAN_STARTED, 0);
				scan_dir *=-1;
				turn_degrees(360*scan_dir, SCAN_SPEED);
				send_message(movement_queue_to_main, MESSAGE_SCAN_COMPLETE, 0);
			break;
			case MESSAGE_HEADING:
				heading = value;
			break;
            case MESSAGE_DROP:
                stop();
                drop_object();
            break;
		}
	}
}
