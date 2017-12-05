#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "movement.h"
#include "messages.h"
#include "sensors.h"

#define Sleep(msec) usleep((msec)*1000)

mqd_t movement_queue, sensors_queue;
pthread_t movement_thread, sensors_thread;
 
void message_handler(uint16_t sensors_command, uint16_t sensors_value) {
    switch (sensors_command) {
        case MSG_SENS_SONAR:
            // if the robot is close to an object, make it turn
            if (sensors_value < 500) {
                send_message(movement_queue, MSG_MOV_TURN, 0);
                printf("Sending message TURN \n");
            }
            else {
                send_message(movement_queue, MSG_MOV_RUN_FORWARD, 0);
                printf("Sending message FORWARD \n");
            }
            break;
    }
}

int main() {

    // Start other modules as threads
    pthread_create(&movement_thread, NULL, movement_start, NULL);
    pthread_create(&sensors_thread, NULL, sensors_start, NULL);

    // Set up the message queues
    movement_queue = init_queue("/movement", O_CREAT | O_WRONLY);
    sensors_queue = init_queue("/sensors", O_CREAT | O_RDONLY);
    uint16_t sensors_command, sensors_value;

    while (1) {
        get_message(sensors_queue, &sensors_command, &sensors_value);
        message_handler(sensors_command, sensors_value);
        // the sleeping makes little sense now as the get_integer function blocks until it has received something.
        // This should probably be changed to nonblocking later though.
        Sleep(10);
    }
}