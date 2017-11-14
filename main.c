#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ev3.h"
#include "movement.h"
#include "messages.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )


int main (){

    if ( ev3_init() == -1 ) return ( 1 );
    messages_init();
    pid_t movement = fork();
    if (movement == 0) { 
        movement_start();
    } else {
        put_integer_in_mq(3);
        Sleep(200);
    }    
}