#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "movement.h"
#include "messages.h"
#include "sensors.h"
#include "bt_client.h"

#define Sleep(msec) usleep((msec)*1000)
#define STATE_TURNING 1
#define STATE_RUNNING 2
#define STATE_SCANNING 3

 mqd_t movement_queue_from_main, movement_queue_to_main, sensors_queue, bluetooth_queue;


int main() {

    movement_init();
    /*
    int sock = bt_connect();
    bt_wait_for_start(sock);
    */
    int state = STATE_TURNING;
    pthread_t movement_thread, sensors_thread;

    pthread_create(&movement_thread, NULL, movement_start, NULL);
    pthread_create(&sensors_thread, NULL, sensors_start, NULL);

    // To make communication bi-directional we should use two queues.
    movement_queue_to_main = init_queue("/movement_to_main", O_CREAT | O_RDONLY | O_NONBLOCK);
    movement_queue_from_main = init_queue("/movement_from_main", O_CREAT | O_WRONLY);
    sensors_queue = init_queue("/sensors", O_CREAT | O_RDONLY);

    uint16_t sensors_command, sensors_value;
    while (1) {
        get_message(sensors_queue, &sensors_command, &sensors_value);
        switch (state) {
            case STATE_TURNING:
                Sleep(500);
                send_message(movement_queue_from_main, MESSAGE_FORWARD, 0);
                state = STATE_RUNNING;
            break;
            
            case STATE_RUNNING:
                if (sensors_command == MESSAGE_SONAR && sensors_value < 500) {
                    send_message(movement_queue_from_main, MESSAGE_TURN, 10);
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