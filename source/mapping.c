#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "messages.h"

#define UNMAPPED 0
#define EMPTY 1
#define OBSTACLE 2
#define MOVABLE 3

char map[80][80] = {UNMAPPED};

void printMap(){
    // We use map[y][x] as in Matlab. We print the map 180 deg flipped for readability
    for (int i = 79; i>=0; i--) {
        for (int j=0; j<80; j++){
            printf("%d", map[i][j]);
        }
        printf("\n");
    }
}

void updateMap(float ang, int dist){
    int x, y;
    for (int i = 0; i < dist; i+=5) {
       y = (i * sin(ang/180 * M_PI))/5;
       x = (i * cos(ang/180 * M_PI))/5;
        if (x + robot_x < 0 || y + robot_y < 0 ) {
            printf("Robot claims to have found clear path at x = %d , y =  %d \n", x, y);
        } else {
            map[y + robot_y][x + robot_x] = 2;
        }
    }
    x = ((dist * cos(ang/180 * M_PI) ) / 5);
    y = ((dist * sin(ang/180 * M_PI) ) / 5);
    if (x + robot_x < 0 || y + robot_y < 0 ) {
        printf("Robot claims to have found clear path at x = %d , y =  %d \n", x, y);
    } else {
        map[y + robot_y][x + robot_x] = 1;
    }
}

void *mapping_start(void* queues){
    mqd_t* tmp = (mqd_t*)queues;
	mqd_t queue_from_main = tmp[0];
    uint16_t command;
	int16_t value, distance, angle;


    while(1) {
        // Get distance and angle
        get_message(queue_from_main, &command, &value);
        if (command == MESSAGE_SONAR) {
            distance = value;
        } else if (command == MESSAGE_GYRO || command == MESSAGE_COMPASS){
            angle = value;
        }
        get_message(queue_from_main, &command, &value);
          if (command == MESSAGE_SONAR) {
            distance = value;
        } else if (command == MESSAGE_GYRO || command == MESSAGE_COMPASS){
            angle = value;
        }
        updateMap((float)angle, distance);
    }
    
}