#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "messages.h"

#define UNMAPPED 0
#define EMPTY 1
#define OBSTACLE 2
#define MOVABLE 3
#define VIRTUAL_WALL 4
#define WALL 5

#define ANGLE_INDEX 0
#define DISTANCE_INDEX 1

char map[80][80] = {UNMAPPED};
int robot_x = 40;
int robot_y = 10;
int16_t data_pair[2];

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
       y = (i * sin(ang/180 * M_PI))/50;
       x = (i * cos(ang/180 * M_PI))/50;
        if (x + robot_x < 0 || y + robot_y < 0 ) {
            //printf("Robot claims to have found clear path at x = %d , y =  %d \n", x, y);
        } else {
            map[y + robot_y][x + robot_x] = EMPTY;
        }
    }
    x = ((dist * cos(ang/180 * M_PI) ) / 50);
    y = ((dist * sin(ang/180 * M_PI) ) / 50);
    if (x + robot_x < 0 || y + robot_y < 0 ) {
       // printf("Robot claims to have found clear path at x = %d , y =  %d \n", x, y);
    } else {
        map[y + robot_y][x + robot_x] = OBSTACLE;
    }
}

void messageHandler(uint16_t command, int16_t value) {
    switch (command) {
        case MESSAGE_POS_X:
            robot_x = value;
            break;
        case MESSAGE_POS_Y:
            robot_y = value;
            break;
        case MESSAGE_ANGLE:
            if (data_pair[0] != -1) {
            printf("MAPPING: Two equal message types (%d) in a row. This should not happen!\n", command);
            }
            data_pair[0] = value;
            break;
        case MESSAGE_SONAR:
            if (data_pair[1] != -1) {
            printf("MAPPING: Two equal message types (%d) in a row. This should not happen!\n", command);
            }
            data_pair[1] = value;
            break;
        case MESSAGE_PRINT_MAP:
            printMap();
            break;
    }
    if (data_pair[0] != -1 && data_pair[1] != -1) {
        updateMap((float)data_pair[0], data_pair[1]);
        data_pair[0] = -1;
        data_pair[1] = -1;
    }
}

void *mapping_start(void* queues){
    mqd_t* tmp = (mqd_t*)queues;
	mqd_t queue_from_main = tmp[0];

    uint16_t command;
    int16_t value;

    while(1) {
        get_message(queue_from_main, &command, &value);
        messageHandler(command, value);
    }
}