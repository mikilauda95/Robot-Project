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

int main() {

    movement_init();
    if (!bt_connect()) {
		exit(1);
	}
	bt_wait_for_start();

	int state = STATE_TURNING;

	queue_sensors_to_main 		= init_queue("/sensors", O_CREAT | O_RDWR);
	queue_main_to_move 			= init_queue("/movement_from_main", O_CREAT | O_RDWR);
	queue_move_to_main 			= init_queue("/movement_to_main", O_CREAT | O_RDWR);
	queue_main_to_bt 			= init_queue("/bt_from_main", O_CREAT | O_RDWR);
	queue_bt_to_main 			= init_queue("/bt_to_main", O_CREAT | O_RDWR);

	// 							V ---- note: movement connected with bluetooth
	mqd_t bt_queues[] = {queue_move_to_main, queue_bt_to_main};
	mqd_t movement_queues[] = {queue_main_to_move, queue_move_to_main};

	pthread_create(&sensors_thread, NULL, sensors_start, NULL);
	pthread_create(&movement_thread, NULL, movement_start, (void*)movement_queues);
	pthread_create(&bluetooth_thread, NULL, bt_client, (void*)bt_queues);

	uint16_t sensors_command, sensors_value;
	while (1) {
		get_message(queue_sensors_to_main, &sensors_command, &sensors_value);
		switch (state) {
			case STATE_TURNING:
				Sleep(500);
				send_message(queue_main_to_move, MESSAGE_FORWARD, 0);
				state = STATE_RUNNING;
			break;
			
			case STATE_RUNNING:
				if (sensors_command == MESSAGE_SONAR && sensors_value < 500) {
					send_message(queue_main_to_move, MESSAGE_TURN, 10);
					state = STATE_TURNING;
				} 
			break;

			case STATE_SCANNING:
			break;
		}
		// the sleeping makes little sense now as the get_integer function blocks until it has received something.
		// This should probably be changed to nonblocking later though.
		Sleep(10);
	}

}