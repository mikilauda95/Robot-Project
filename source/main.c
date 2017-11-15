#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ev3.h"
#include "movement.h"
#include "messages.h"
#include "sensors.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )

void message_handler(mqd_t movement_queue, mqd_t sensors_queue, int sensors_command, int sensors_value) {
    switch (sensors_command) {
        case MESSAGE_SONAR:
            // if the robot is close to an object, make it turn
            if (sensors_value < 500) {
                put_integer_in_mq(movement_queue, MESSAGE_TURN, 0); 
                printf("Sending message TURN \n");
            } else {
                put_integer_in_mq(movement_queue, MESSAGE_FORWARD, 0);
                printf("Sending message FORWARD \n"); 
            } 
            break;
    }
}

int main (){

    ev3_init();

    pid_t movement = fork();
    if (movement == 0) { 
        movement_start();
    } else {

        pid_t sensors = fork();
        if (sensors ==0) {
            sensors_start();
        } else {
            mqd_t movement_queue = init_queue("/movement", O_CREAT | O_WRONLY);
            mqd_t sensors_queue = init_queue("/sensors", O_CREAT | O_RDONLY);
            
            int sensors_command, sensors_value;
            while (1){
                get_integer_from_mq(sensors_queue, &sensors_command, &sensors_value);
                message_handler(movement_queue, sensors_queue, sensors_command, sensors_value);
                // the sleeping makes little sense now as the get_integer function blocks until it has received something.
                // This should probably be changed to nonblocking later though. 
                Sleep(10);
            }
            
        }
    }    
}