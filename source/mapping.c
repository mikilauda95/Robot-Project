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

#define MAP_SIZE_X 80
#define MAP_SIZE_Y 80
#define MAX_DIST 800 // Max distance in mm

int8_t map[MAP_SIZE_Y][MAP_SIZE_X] = {UNMAPPED};
int robot_x = 40;
int robot_y = 10;
int16_t data_pair[2];
int16_t pos_pair[2] = {-1, -1};


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
    y = (dist * sin(ang/180 * M_PI))/50;
    x = (dist * cos(ang/180 * M_PI))/50;
    if (robot_x + x < 0 || robot_x + x >= MAP_SIZE_X || robot_y + y < 0 || robot_y + y >= MAP_SIZE_Y) {
        //printf("Mapping: Got positions outside of map!\n");
        return;
    } else {

    }
    for (int i = 0; i < (dist>MAX_DIST?MAX_DIST:dist); i+=50) {
        y = (i * sin(ang/180 * M_PI))/50;
        x = (i * cos(ang/180 * M_PI))/50;
        if (map[y + robot_y][x + robot_x] == UNMAPPED) {
            map[y + robot_y][x + robot_x] = EMPTY;
        }
    }
    if (dist < MAX_DIST) {
        x = (dist * cos(ang/180 * M_PI)) / 50;
        y = (dist * sin(ang/180 * M_PI)) / 50;
        map[y + robot_y][x + robot_x] = OBSTACLE;
    }
    map[robot_y][robot_x] = 7;
}

void messageHandler(uint16_t command, int16_t value) {
    switch (command) {
        case MESSAGE_POS_X:
        case MESSAGE_POS_Y:
            // These lines are to make sure that we have update both positions at the same time.
            pos_pair[command==MESSAGE_POS_X?0:1] = value;
            if (pos_pair[0] != -1 && pos_pair[1] != -1) {
                robot_x = pos_pair[0];
                robot_y = pos_pair[1];
                pos_pair[0] = -1;
                pos_pair[1] = -1;
            }
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