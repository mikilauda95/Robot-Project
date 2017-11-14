#include <stdio.h>
#include "movement.h"
#include "ev3.h"


int main (){

    if ( ev3_init() == -1 ) return ( 1 );
    movement_init();
    movement_start();
    
}