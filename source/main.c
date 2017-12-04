#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ev3.h"
#include "movement.h"
#include "messages.h"
#include "sensors.h"

#define Sleep(msec) usleep((msec)*1000)

 mqd_t movement_queue, sensors_queue, color_queue;

void color_handler(uint16_t color_command, uint16_t color_value) {
    switch (color_command)
    {
    case MESSAGE_COLOR:
        switch (color_value) {//to be completed
            case 1:
                printf("THIS IS RED\n");
                break;
            default:
                break;
        }
        // if the robot is close to an object, make it turn
    }
}
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
    ev3_init();

    pid_t movement = fork();
    if (movement == 0)
    {
        movement_start();
    }
    else
    {

        pid_t sensors = fork();
        if (sensors == 0)
        {
            sensors_start();
        }
        else
        {
            pause();
        }
    }
}
