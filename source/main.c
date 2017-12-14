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

mqd_t movement_queue_from_main, movement_queue_to_main, sensors_queue,  bt_from_main_queue, bt_to_main_queue;

int main() {

    movement_init();
    /*
    if (!bt_connect()) {
		exit(1);
	}
	bt_wait_for_start();
    */
    int state;
    pthread_t movement_thread, sensors_thread, bluetooth_thread;

    // To make communication bi-directional we should use two queues.
    movement_queue_to_main = init_queue("/movement_to_main", O_CREAT | O_RDONLY | O_NONBLOCK);
    movement_queue_from_main = init_queue("/movement_from_main", O_CREAT | O_WRONLY);
    sensors_queue = init_queue("/sensors", O_CREAT | O_RDONLY);
    bt_from_main_queue = init_queue("/bt_from_main", O_CREAT | O_RDWR);
	bt_to_main_queue = init_queue("/bt_to_main", O_CREAT | O_RDWR);

    mqd_t bt_queues[] = {bt_from_main_queue, bt_to_main_queue};

    pthread_create(&movement_thread, NULL, movement_start, NULL);
    pthread_create(&sensors_thread, NULL, sensors_start, NULL);
    pthread_create(&bluetooth_thread, NULL, bt_client, (void*)bt_queues);

    uint16_t sensors_command, sensors_value;
    uint16_t movement_command, movement_value;

    bool got_mov_msg = false;

    // start robot in forward state. In the real program it should start with scanning. 
    send_message(movement_queue_from_main, MESSAGE_FORWARD, 0); 
    state = STATE_RUNNING;

    while (1) {
        get_message(sensors_queue, &sensors_command, &sensors_value);
        if (get_message(movement_queue_to_main, &movement_command, &movement_value) != -1) {
            got_mov_msg = true;
        }
        switch (state) {
            case STATE_TURNING:
                if (got_mov_msg == true && movement_command == MESSAGE_TURN_COMPLETE) {
                    send_message(movement_queue_from_main, MESSAGE_FORWARD, 0);
                    state = STATE_RUNNING;
                }
                
            break;
            
            case STATE_RUNNING:
                if (sensors_command == MESSAGE_SONAR && sensors_value < 500) {
                    send_message(movement_queue_from_main, MESSAGE_TURN, 90);
                    state = STATE_TURNING;
                } 
            break;

            case STATE_SCANNING:
            break;
        }
        got_mov_msg = false;
        // the sleeping makes little sense now as the get_integer function blocks until it has received something.
        // This should probably be changed to nonblocking later though.
        Sleep(10);
    }
}