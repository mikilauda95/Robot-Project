#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
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

void wait_for_queues(uint16_t *command, uint16_t *value) {

	static int current_queue_index = 0;
	
	mqd_t read_queues[] = {
		queue_sensors_to_main,
		queue_move_to_main,
		queue_bt_to_main
	};
	uint8_t n_queues = sizeof(read_queues) / sizeof(mqd_t);
	current_queue_index %= n_queues;

	for(;;) {
		int ret = get_message(read_queues[current_queue_index++], command, value);
		current_queue_index %= n_queues;
		if (ret != -1){
			printf("Queue %d had values %d, %d\n", current_queue_index, *command, *value);
			return;
		}
	}

}

void event_handler(uint16_t *command, uint16_t *value) {
	static int state = STATE_TURNING;

	// handle events not depending on current state	
	switch(*command) {
		case MESSAGE_POS_X:
		case MESSAGE_POS_Y:
			send_message(queue_main_to_bt, *command, *value);
	}

	// handle events depending on current state
	switch (state) {
		case STATE_TURNING:
			printf("turning\n");
			// Sleep(500);
			// send_message(queue_main_to_move, MESSAGE_FORWARD, 0);
			// state = STATE_RUNNING;
		break;
		
		case STATE_RUNNING:
			if (*command == MESSAGE_SONAR && *value < 500) {
				send_message(queue_main_to_move, MESSAGE_TURN, 10);
				state = STATE_TURNING;
			} 
		break;

		case STATE_SCANNING:
		break;
	}
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

	pthread_create(&sensors_thread, NULL, sensors_start, NULL);
	pthread_create(&movement_thread, NULL, movement_start, (void*)movement_queues);
	pthread_create(&bluetooth_thread, NULL, bt_client, (void*)bt_queues);

	uint16_t command, value;

	for(;;){
		wait_for_queues(&command, &value);
		event_handler(&command, &value);
	}

}