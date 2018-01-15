#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#include "movement.h"
#include "messages.h"
#include "sensors.h"
#include "bt_client.h"
#include "mapping.h"
#include "tuning.h"

#define Sleep(msec) usleep((msec)*1000)
#define STATE_TURNING 1
#define STATE_RUNNING 2
#define STATE_SCANNING 3
#define STATE_STOPPED 4
#define STATE_DROP 5
#define STATE_DONE 6

mqd_t queue_main_to_move, queue_move_to_main;
mqd_t queue_main_to_bt, queue_bt_to_main;
mqd_t queue_sensors_to_main;
mqd_t queue_main_to_mapping, queue_mapping_to_main;

pthread_t sensors_thread, movement_thread, bluetooth_thread, mapping_thread;
int target_heading = START_HEADING; // Start facing forward
int current_heading;
int state;

int no_route_found_count = 0;
int scan_count = 0;

void wait_for_queues(uint16_t *command, int16_t *value) {

	static int current_queue_index = 0;
	
	mqd_t read_queues[] = {
		queue_sensors_to_main,
		queue_move_to_main,
		queue_bt_to_main,
		queue_mapping_to_main
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
			send_message(queue_main_to_bt, command, value);
			send_message(queue_main_to_mapping, command, value);
			break;
		case MESSAGE_ANGLE:
			// We receive the angle in centidegrees. But main will work with degrees
			current_heading = (int)(value/10.0 + 0.5);
			break;
		case MESSAGE_NO_CLEAR_ROUTE_FOUND:
			no_route_found_count++;
			if(no_route_found_count == SEARCHES_BEFORE_MAP_SEND) {
				printf("Scan count %d, sending map...\n", scan_count);
				send_message(queue_main_to_mapping, MESSAGE_SEND_MAP, 0);
				state = STATE_DONE;
			}
			break;
	}

	// handle events depending on current state
	switch (state) {
		case STATE_TURNING:
			if (command == MESSAGE_TURN_COMPLETE) {
				if (target_heading != current_heading) {
					int delta = (target_heading - current_heading);
					if (delta < -180) {
						delta +=360;
					} else if (delta > 180) {
						delta -=360;
					}
					//printf("Turn not complete. Delta: %d, curr: %d, target: %d \n", delta, current_heading, target_heading);
					send_message(queue_main_to_move, MESSAGE_TURN_DEGREES, delta);
				} else {
					//printf("Turn complete! \n");
					send_message(queue_main_to_move, MESSAGE_HEADING, current_heading);
					if (scan_count == SCANS_BEFORE_RELEASE) {
						send_message(queue_main_to_move, MESSAGE_DROP, 0);
						state = STATE_DROP;
					} else {
						send_message(queue_main_to_move, MESSAGE_FORWARD, 0);
						state = STATE_RUNNING;
					} 
				}
			}
			
		break;
		
		case STATE_RUNNING:
			if (command == MESSAGE_SONAR && value < 250) {
				printf("Main: stop. Reason: sonar (value %d)\n", value);
				send_message(queue_main_to_move, MESSAGE_STOP, 0);
				send_message(queue_main_to_move, MESSAGE_SCAN, 0);
				state = STATE_STOPPED;
			} else if (command == MESSAGE_REACHED_DEST) {
				printf("Main: stop. Reason: reached target distance\n");
				send_message(queue_main_to_move, MESSAGE_STOP, 0);
				send_message(queue_main_to_move, MESSAGE_SCAN, 0);
				state = STATE_STOPPED;
			}
		break; 

		case STATE_SCANNING:
			if (command == MESSAGE_SCAN_COMPLETE) {
				send_message(queue_main_to_mapping, MESSAGE_PRINT_MAP, 0);
				send_message(queue_main_to_move, MESSAGE_STOP, 0);
				send_message(queue_main_to_mapping, MESSAGE_SCAN_COMPLETE, 0);
				scan_count++;
				state = STATE_STOPPED;
			} else if (command == MESSAGE_ANGLE || command == MESSAGE_SONAR) {
				// When scanning, forward angle and distance. If these are not alternating, something is wrong
				send_message(queue_main_to_mapping, command, value);
			}
		break;

		case STATE_DROP:
			switch (command) {
				case MESSAGE_DROP_X:
				case MESSAGE_DROP_Y:
					send_message(queue_main_to_mapping, command, value);
					send_message(queue_main_to_bt, command, value);
				break;
				case MESSAGE_DROP_COMPLETE:
					printf("drop complete\n");
					send_message(queue_main_to_move, MESSAGE_TARGET_DISTANCE, 500);
					send_message(queue_main_to_move, MESSAGE_FORWARD, 0);
					state = STATE_RUNNING;	
				break;
			}
		break;

		case STATE_STOPPED:
			switch (command) {
				case MESSAGE_SCAN_STARTED:
					send_message(queue_main_to_mapping, MESSAGE_SCAN, 0);
					state = STATE_SCANNING;
				break;
				case MESSAGE_TARGET_DISTANCE:
					send_message(queue_main_to_move, MESSAGE_TARGET_DISTANCE, value);
				break;
				case MESSAGE_TARGET_ANGLE:
				{
					int delta = (value - current_heading);
					printf("got new target angle: %d => delta = %d\n", value, delta);
					if (delta < -180) {
						delta +=360;
					} else if (delta > 180) {
						delta -=360;
					}
					target_heading = value;
					send_message(queue_main_to_move, MESSAGE_TURN_DEGREES, delta);
					state = STATE_TURNING;
				}
				break;
			}
		break;

		case STATE_DONE:
			// do nothing.
			for(;;);
		break;

	}
}

// handler for ctrl+c
void  INThandler() {

	send_message(queue_main_to_move, MESSAGE_STOP, 0);
	mq_close(queue_sensors_to_main);

	pthread_cancel(sensors_thread);

	printf("Caught ctrl+c, sending map...\n");
	send_message(queue_main_to_mapping, MESSAGE_SEND_MAP, 0);

	Sleep(500);
	pthread_cancel(movement_thread);

	for(int i = 0; i < 30; i++) {
		Sleep(1000);
	}

	printf("done sending map, killing remaining threads.\n");

	// Let the movement thread have some time to stop motors

	// Close and unlink the queues, this should help making sure we have no messages left from the previous run
	mq_close(queue_main_to_bt);
	mq_close(queue_bt_to_main);
	mq_close(queue_main_to_move);
	mq_close(queue_move_to_main);
	mq_close(queue_main_to_mapping);
	mq_close(queue_mapping_to_main);
	
	mq_unlink("/sensors");
	mq_unlink("/movement_from_main");
	mq_unlink("/movement_to_main");
	mq_unlink("/bt_from_main");
	mq_unlink("/bt_to_main");
	mq_unlink("/main_to_mapping");
	mq_unlink("/mapping_to_main");


	pthread_cancel(bluetooth_thread);
	pthread_cancel(mapping_thread);
	exit(0);
}

int main() {

    movement_init();

	if (!bt_connect()) {
		exit(1);
	}
	bt_wait_for_start();
	
	queue_sensors_to_main 		= init_queue("/sensors", O_CREAT | O_RDWR | O_NONBLOCK);
	queue_main_to_move 			= init_queue("/movement_from_main", O_CREAT | O_RDWR);
	queue_move_to_main 			= init_queue("/movement_to_main", O_CREAT | O_RDWR | O_NONBLOCK);
	queue_main_to_bt 			= init_queue("/bt_from_main", O_CREAT | O_RDWR);
	queue_bt_to_main 			= init_queue("/bt_to_main", O_CREAT | O_RDWR | O_NONBLOCK);
	queue_main_to_mapping 		= init_queue("/main_to_mapping", O_CREAT | O_RDWR);
	queue_mapping_to_main 		= init_queue("/mapping_to_main", O_CREAT | O_RDWR | O_NONBLOCK);


	mqd_t bt_queues[] = {queue_main_to_bt, queue_bt_to_main};
	mqd_t movement_queues[] = {queue_main_to_move, queue_move_to_main};
	mqd_t sensor_queues[] = {queue_sensors_to_main};
	mqd_t mapping_queues[] = {queue_main_to_mapping, queue_mapping_to_main, queue_main_to_bt};

	pthread_create(&sensors_thread, NULL, sensors_start, (void*)sensor_queues);
	pthread_create(&movement_thread, NULL, movement_start, (void*)movement_queues);
	pthread_create(&mapping_thread, NULL, mapping_start, (void*)mapping_queues);

	pthread_create(&bluetooth_thread, NULL, bt_client, (void*)bt_queues);

	signal(SIGINT, INThandler); // Setup INThandler to run on ctrl+c

	// Start by scanning in the small arena. Start by running forward in the big one
	if (STADIUM_TYPE == 0) {
		send_message(queue_main_to_move, MESSAGE_SCAN, 0);
		state = STATE_SCANNING;	
	} else {
		send_message(queue_main_to_move, MESSAGE_TARGET_DISTANCE, 500);
		send_message(queue_main_to_move, MESSAGE_FORWARD, 0);
		state = STATE_RUNNING;
	}

	uint16_t command;
	int16_t value;
	for(;;){
		
		wait_for_queues(&command, &value);
		event_handler(command, value);
		
	}
}
