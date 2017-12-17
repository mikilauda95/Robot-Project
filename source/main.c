#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "movement.h"
#include "messages.h"
#include "sensors.h"
#include "bt_client.h"

#define Sleep(msec) usleep((msec)*1000)
#define STATE_TURNING 1
#define STATE_RUNNING 2
#define STATE_SCANNING 3

mqd_t queue_main_to_move, queue_move_to_main;
mqd_t queue_main_to_bt, queue_bt_to_main;
mqd_t queue_sensors_to_main;
pthread_t sensors_thread, movement_thread, bluetooth_thread;

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
	static int state = STATE_RUNNING;
	
	// handle events not depending on current state	
	switch(command) {
		case MESSAGE_POS_X:
		case MESSAGE_POS_Y:
			send_message(queue_main_to_bt, command, value);
			return;
		break;
	}

	// handle events depending on current state
	switch (state) {
		case STATE_TURNING:

			if (command == MESSAGE_GYRO) {
				send_message(queue_main_to_move, command, value);

			}else if (command == MESSAGE_TURN_COMPLETE) {
				send_message(queue_main_to_move, MESSAGE_FORWARD, 0);
				state = STATE_RUNNING;
			}
			
		break;
		
		case STATE_RUNNING:
			if (command == MESSAGE_SONAR) {
				if (value < 500) {
					
					send_message(queue_main_to_move, MESSAGE_TURN_DEGREES, -90);
					state = STATE_TURNING;
					return;
				}
			} 

		case STATE_SCANNING:
		break;
	}
}

// handler for ctrl+c
void  INThandler() {
	printf("Caught ctrl+c, sending stop message to movement_thread\n");
	send_message(queue_main_to_move, MESSAGE_STOP, 0);
	// Let the movement thread have some time to stop motors
	Sleep(1000);
	pthread_cancel(sensors_thread);
	pthread_cancel(movement_thread);
	pthread_cancel(bluetooth_thread);
	
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

	mqd_t bt_queues[] = {queue_main_to_bt, queue_bt_to_main};
	mqd_t movement_queues[] = {queue_main_to_move, queue_move_to_main};
	mqd_t sensor_queues[] = {queue_sensors_to_main};

	pthread_create(&sensors_thread, NULL, sensors_start, (void*)sensor_queues);
	pthread_create(&movement_thread, NULL, movement_start, (void*)movement_queues);
	pthread_create(&bluetooth_thread, NULL, bt_client, (void*)bt_queues);

	signal(SIGINT, INThandler); // Setup INThandler to run on ctrl+c

	send_message(queue_main_to_move, MESSAGE_FORWARD, 0);	

	uint16_t command;
	int16_t value;
	for(;;){
		wait_for_queues(&command, &value);
		event_handler(command, value);
	}
}