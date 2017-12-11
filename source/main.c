#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ev3.h"
#include "movement.h"
#include "messages.h"
#include "sensors.h"
#include "bt_client.h"

#define Sleep(msec) usleep((msec)*1000)

 mqd_t movement_queue, sensors_queue, bluetooth_queue;

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
            pid_t bluetooth = fork();
            if (bluetooth == 0)
            {
                bluetooth_queue = init_queue("/bluetooth", O_CREAT | O_RDWR);
                printf("Trying to connect...\r\n");

                int sock = bt_connect();
                if (bt_wait_for_start(sock)){
                    printf("Did not receive start\r\n");
                }else{
                    printf("Did receive start\r\n");
                }

                bt_send_position(sock);
            }
            else
            {
                movement_queue = init_queue("/movement", O_CREAT | O_WRONLY);
                sensors_queue = init_queue("/sensors", O_CREAT | O_RDONLY);

                uint16_t sensors_command, sensors_value;
                while (1)
                {
                    get_message(sensors_queue, &sensors_command, &sensors_value);
                    message_handler(sensors_command, sensors_value);
                    // the sleeping makes little sense now as the get_integer function blocks until it has received something.
                    // This should probably be changed to nonblocking later though.
                    Sleep(10);
                }
            }
        }
    }
}