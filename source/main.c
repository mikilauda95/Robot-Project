#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "movement.h"
#include "messages.h"
#include "sensors.h"

#define Sleep(msec) usleep((msec)*1000)
#define STATE_TURNING 1
#define STATE_RUNNING 2
#define STATE_SCANNING 3

 mqd_t movement_queue, sensors_queue;

void message_handler(uint16_t sensors_command, uint16_t sensors_value) {
    switch (sensors_command)
    {
    case MESSAGE_SONAR:
        // if the robot is close to an object, make it turn
        if (sensors_value < 500)
        {
            send_message(movement_queue, MESSAGE_TURN, 0);
            printf("Sending message TURN \n");
        }
        else
        {
            send_message(movement_queue, MESSAGE_FORWARD, 0);
            printf("Sending message FORWARD \n");
        }
        break;
    }
}

int main() {
    int state = STATE_TURNING;
    pthread_t movement_thread, sensors_thread;

    pthread_create(&movement_thread, NULL, movement_start, NULL);
    pthread_create(&sensors_thread, NULL, sensors_start, NULL);

    movement_queue = init_queue("/movement", O_CREAT | O_WRONLY);
    sensors_queue = init_queue("/sensors", O_CREAT | O_RDONLY);

    uint16_t sensors_command, sensors_value;
    while (1) {
        get_message(sensors_queue, &sensors_command, &sensors_value);
        switch (state) {
            case STATE_TURNING:
                Sleep(500);
                send_message(movement_queue, MESSAGE_FORWARD, 0);
                state = STATE_RUNNING;
            break;
            
            case STATE_RUNNING:
                if (sensors_command == MESSAGE_COMPASS && sensors_value < 500) {
                    send_message(movement_queue, MESSAGE_TURN, 10);
                    state = STATE_TURNING;
                } 
            break;

            case STATE_SCANNING:
            break;
        }
        // message_handler(sensors_command, sensors_value);
        // the sleeping makes little sense now as the get_integer function blocks until it has received something.
        // This should probably be changed to nonblocking later though.
        Sleep(10);
    }

}