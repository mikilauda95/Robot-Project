#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "ev3.h"
#include "movement.h"
#include "messages.h"
#include "sensors.h"

#define Sleep(msec) usleep((msec)*1000)

mqd_t movement_queue, sensors_queue, mapping_queue;

int END;

void message_handler(uint16_t sensors_command, uint16_t sensors_value) {
    uint16_t tmp_value, temp_command;
    printf("received a value from the sensor queue here in main!!\n");
    switch (sensors_command)
    {
        case MESSAGE_SONAR:
            // if the robot is close to an object, make it turn
            printf("sensor value is %d\n",sensors_value);
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

        case MESSAGE_AVGCOMPASS:
            send_message(sensors_queue, MESSAGE_AVGCOMPASS, 0);
            get_message(sensors_queue, &sensors_command, &tmp_value);
            /*send_message(mapping_queue, MESSAGE_AVGCOMPASS, tmp_value);//could be substituted with a forward*/
            send_message(movement_queue, MESSAGE_CONTINUE, 0);//could be substituted with a forward
            /*get_message(mapping_queue, &mapping_command, &mapping_value);*/

            break;

        case MESSAGE_END_SCANN:
            END=1;
            break;
    }
}


int main(int argc, char *argv[]) {
    int option;
    option=atoi(argv[1]);
    printf("ATOI OF ARG1 IS %d\n",option);
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
            printf("sensors have been started\n");
            sensors_start();
        }
        else
        {
            //modified to make work some function
            movement_queue = init_queue("/movement", O_CREAT | O_RDWR);
            sensors_queue = init_queue("/sensors", O_CREAT | O_RDWR);
            /*mapping_queue = init_queue("/mapping", O_CREAT | O_RDWR);*/

            /*movement_queue = init_queue("/movement", O_CREAT | O_WRONLY);*/
            /*sensors_queue = init_queue("/sensors", O_CREAT | O_RDWR);*/
            /*mapping_queue = init_queue("/mapping", O_CREAT | O_RDWR);*/

            uint16_t sensors_command, sensors_value;
            uint16_t movement_command, movement_value;
            uint16_t mapping_command, mapping_value;
            //usage

            int FINISHED=1;
            while (FINISHED)
            {
                switch (option) {
                    case 1:
                        {
                            //testing scanning function
                            //testing scanning function
                            END=0;
                            int num_scan=10;
                            send_message(movement_queue, MESSAGE_SCANN,num_scan);
                            send_message(mapping_queue, MESSAGE_SCANN,num_scan);
                            while (!END) { //END IS SET FROM message_handler when it receives the ending
                                get_message(movement_queue, &movement_command, &movement_value);
                                message_handler(sensors_command, sensors_value);
                            }
                            FINISHED=0;
                        }
                        break;
                    case 2:
                        {
                            //testing the turning function
                            //testing relative position
                            send_message(movement_queue, MESSAGE_TURNDEGREES,0);
                            FINISHED=0;
                        }
                        break;
                    case 3:
                        {
                            send_message(sensors_queue, MESSAGE_COLORSENS, 0);
                            get_message(sensors_queue, &sensors_value, &sensors_command);
                            printf("the color value read is %d\n", sensors_value);
                            FINISHED=0;
                        }
                        break;
                    default:
                        {
                            /*send_message(movement_queue, MESSAGE_START, 0);*/
                            send_message(sensors_queue, MESSAGE_START, 0);
                            while (1){
                                get_message(sensors_queue, &sensors_command, &sensors_value);
                                message_handler(sensors_command, sensors_value);
                                // the sleeping makes little sense now as the get_integer function blocks until it has received something.
                                // This should probably be changed to nonblocking later though.
                                Sleep(10);
                            }
                            FINISHED=0;
                        }
                }
            }
        }
    }
}
