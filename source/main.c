#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "movement.h"
#include "messages.h"
#include "sensors.h"

#define Sleep(msec) usleep((msec)*1000)
#define STATE_SCANNING 1
#define STATE_RUNNING 2
#define STATE_TURNING 3

mqd_t movement_queue, sensors_queue;
pthread_t movement_thread, sensors_thread;

int state = STATE_SCANNING;
 
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

// TODO: Find out wheather the message queues can work bi-directionally, or if we should have one per direciton. 
// Also, the movement_queue should be non blocking in main, meaning main should be able to check for a message without beeing blocked.
// Perhaps the solution is to make none of the queues blocking?

int main() {

    // Start other modules as threads
    pthread_create(&movement_thread, NULL, movement_start, NULL);
    pthread_create(&sensors_thread, NULL, sensors_start, NULL);

    // Set up the message queues
    movement_queue = init_queue("/movement", O_CREAT | O_RDWR);
    sensors_queue = init_queue("/sensors", O_CREAT | O_RDONLY);
    uint16_t sensors_command, sensors_value;
    uint16_t movement_command, movement_value;

    while (1) {
        // Get messages. Need to keep on emptying the queue
        get_message(sensors_queue, &sensors_command, &sensors_value);
        get_message(movement_queue, &movement_command, &movement_value);
        
        switch (state) {
            case STATE_SCANNING:
                
                break;

            case STATE_RUNNING:
                break;

            case STATE_TURNING:
                break;
        }
        message_handler(sensors_command, sensors_value);
    }
}