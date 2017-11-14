#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ev3.h"
#include "movement.h"
#include "messages.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )


int main (){

    if ( ev3_init() == -1 ) return ( 1 );

    pid_t movement = fork();
    if (movement == 0) { 
        movement_start();
    } else {
        mqd_t movement_queue = init_queue("/movement", O_CREAT | O_WRONLY);
        put_integer_in_mq(movement_queue, SET_SPEED * 2048 + 599);
        Sleep(200);
    }    
}