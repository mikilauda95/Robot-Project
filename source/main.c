#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "movement.h"
#include "messages.h"
#include "sensors.h"
#include "mapping.h"
#include "bt_client.h"

#define Sleep(msec) usleep((msec)*1000)
#define STATE_TURNING 1
#define STATE_RUNNING 2
#define STATE_SCANNING 3

mqd_t queue_main_to_move, queue_move_to_main;
mqd_t queue_main_to_bt, queue_bt_to_main;
mqd_t queue_sensors_to_main;
mqd_t queue_main_to_mapping;
pthread_t sensors_thread, movement_thread, bluetooth_thread, mapping_thread;
int target_heading = 90; // Start facing forward
int current_heading;
int state = STATE_SCANNING;

void wait_for_queues(uint16_t *command, int16_t *value) {

	static int current_queue_index = 0;
	
	mqd_t read_queues[] = {
		queue_sensors_to_main,
		queue_move_to_main,
		queue_bt_to_main
	};
	uint8_t n_queues = sizeof(read_queues) / sizeof(mqd_t);
	current_queue_index %= n_queues;

	for(;;) {
		int ret = get_message(read_queues[current_queue_index], command, value);
		current_queue_index = (current_queue_index + 1) % n_queues;
		if (ret != -1){
			return;
		}
	}

}


void event_handler(uint16_t command, int16_t value) {
	// handle events not depending on current state	
	switch(command) {
		case MESSAGE_POS_X:
		case MESSAGE_POS_Y:
			//send_message(queue_main_to_bt, command, value);
			send_message(queue_main_to_mapping, command, value);
			break;
		case MESSAGE_ANGLE:
			current_heading = value;
			break;
	}

	// handle events depending on current state
	switch (state) {
		case STATE_TURNING:

			if (command == MESSAGE_TURN_COMPLETE) {
				if (target_heading != current_heading) {
					int delta = (target_heading - current_heading); // TODO consider moving this stuff to movement module, and let main operate with heading over 360 and under 0
					if (delta < -180) {
						delta +=360;
					} else if (delta > 180) {
						delta -=360;
					}
					//printf("Turn not complete. Delta: %d, curr: %d, target: %d \n", delta, current_heading, target_heading);
					send_message(queue_main_to_move, MESSAGE_TURN_DEGREES, delta);
				} else {
					printf("Turn complete! \n");
					send_message(queue_main_to_move, MESSAGE_FORWARD, 0);
					state = STATE_RUNNING;
				}
			}
			
		break;
		
		case STATE_RUNNING:
			if (command == MESSAGE_SONAR) {
				if (value < 300) {
					
					send_message(queue_main_to_move, MESSAGE_TURN_DEGREES, -90);
					target_heading -= 90;
					if (target_heading < 0){target_heading+=360;}
					state = STATE_TURNING;
				}
			} 
		break;

		case STATE_SCANNING:
			if (command == MESSAGE_SCAN_COMPLETE) {
				send_message(queue_main_to_mapping, MESSAGE_PRINT_MAP, 0);
				// turn the robot where we want it before running again
				int delta = (target_heading - current_heading); // TODO consider moving this stuff to movement module, and let main operate with heading over 360 and under 0
				if (delta < -180) {
					delta +=360;
				} else if (delta > 180) {
					delta -=360;
				}
				send_message(queue_main_to_move, MESSAGE_TURN_DEGREES, delta);
				state = STATE_TURNING;
			} else if (command == MESSAGE_ANGLE || command == MESSAGE_SONAR) {
				// When scanning, forward angle and distance. If these are not alternating, something is wrong
				send_message(queue_main_to_mapping, command, value);
			}

		break;
	}
}

// handler for ctrl+c
void INThandler() {
	printf("Caught ctrl+c, sending stop message to movement_thread\n");
	send_message(queue_main_to_move, MESSAGE_STOP, 0);
	// Let the movement thread have some time to stop motors
	Sleep(500);
	pthread_cancel(sensors_thread);
	pthread_cancel(movement_thread);
	pthread_cancel(bluetooth_thread);
	pthread_cancel(mapping_thread);
	
	exit(0);
}

int main() {

    movement_init();
	/*
	if (!bt_connect()) {
		exit(1);
	}
	bt_wait_for_start();
	*/
	
	queue_sensors_to_main 		= init_queue("/sensors2", O_CREAT | O_RDWR | O_NONBLOCK);
	queue_main_to_move 			= init_queue("/movement_from_main2", O_CREAT | O_RDWR);
	queue_move_to_main 			= init_queue("/movement_to_main2", O_CREAT | O_RDWR | O_NONBLOCK);
	queue_main_to_bt 			= init_queue("/bt_from_main2", O_CREAT | O_RDWR);
	queue_bt_to_main 			= init_queue("/bt_to_main2", O_CREAT | O_RDWR | O_NONBLOCK);
	queue_main_to_mapping 		= init_queue("/main_to_mapping2", O_CREAT | O_RDWR);

	mqd_t bt_queues[] = {queue_main_to_bt, queue_bt_to_main};
	mqd_t movement_queues[] = {queue_main_to_move, queue_move_to_main};
	mqd_t sensor_queues[] = {queue_sensors_to_main};
	mqd_t mapping_queues[] = {queue_main_to_mapping};

	//pthread_create(&sensors_thread, NULL, sensors_start, (void*)sensor_queues);
	//pthread_create(&movement_thread, NULL, movement_start, (void*)movement_queues);
	pthread_create(&mapping_thread, NULL, mapping_start, (void*)mapping_queues);
	//pthread_create(&bluetooth_thread, NULL, bt_client, (void*)bt_queues);

	signal(SIGINT, INThandler); // Setup INThandler to run on ctrl+c

	//send_message(queue_main_to_move, MESSAGE_SCAN, 0);	

	uint16_t command;
	int16_t value;
	for(;;){
		
		//wait_for_queues(&command, &value);
		//event_handler(command, value);
		
	}
}