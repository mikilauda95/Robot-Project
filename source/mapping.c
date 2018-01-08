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
#define MAX_DIST 500 // Max distance in mm
#define TILE_SIZE 50.0 // Size of each tile in mm. With decimal to ensure float division
#define SONAR_OFFSET 100 // Distance from rotation axis to the sonar in mm

uint8_t map[MAP_SIZE_Y][MAP_SIZE_X] = {UNMAPPED};

int robot_x = 2000; // robot start position in mm
int robot_y = 1000;

int16_t data_pair[2] = {-1, -1};
int16_t pos_pair[2] = {-1, -1};

FILE * f;

void printMap(){
    // We use map[y][x] as in Matlab. We print the map 180 deg flipped for readability
    for (int i = MAP_SIZE_Y-1; i>=0; i--) {
        for (int j=0; j<MAP_SIZE_X; j++){
            printf("%d", map[i][j]);
        }
        printf("\n");
    }
}

void update_map(float ang, int dist){
    int x, y;   
    for (int i = 0; i < (dist>MAX_DIST?MAX_DIST:dist); i+=TILE_SIZE) {
        y = (int)((((i+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((i+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);

        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {
            // Return if a value is out of the map. No need to try the other values
            return;
        } else if (map[y][x] == UNMAPPED) {
            map[y][x] = EMPTY;
        }
    }
    if (dist < MAX_DIST) {
        y = (int)((((dist+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((dist+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);
        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {  
            return;
        }
        map[y][x] = OBSTACLE;
        fprintf(f, "%d %d\n", x, y);
    }
    map[(int)(robot_y/TILE_SIZE + 0.5)][(int)(robot_x/TILE_SIZE +0.5)] = 7;
}

void message_handler(uint16_t command, int16_t value) {
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
        case MESSAGE_SONAR:
            data_pair[command==MESSAGE_ANGLE?0:1] = value;
            if (data_pair[0] != -1 && data_pair[1] != -1) {
                update_map((float)data_pair[0], data_pair[1]);
                data_pair[0] = -1;
                data_pair[1] = -1;
            }
            break;
        case MESSAGE_PRINT_MAP:
            printMap();
            break;
    }
}

void *mapping_start(void* queues){
    mqd_t* tmp = (mqd_t*)queues;
	mqd_t queue_from_main = tmp[0];
    f = fopen("objects.txt", "w");

    uint16_t command;
    int16_t value;

    while(1) {
        get_message(queue_from_main, &command, &value);
        message_handler(command, value);
    }
}