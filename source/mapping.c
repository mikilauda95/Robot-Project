#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "mapping.h"
#include "messages.h"
#include "tuning.h"

#define MAX_INCREMENTS 31
// 1-W indicates objects with an increasing level of certainty
char *printlist = "* r'?X????123456789ABCDEFGHIJKLMNOPQRSTUVW";

uint8_t map[MAP_SIZE_Y][MAP_SIZE_X] = {UNMAPPED};

int robot_x = ROBOT_START_X; // robot start position in mm
int robot_y = ROBOT_START_Y;

int16_t data_pair[2] = {-1, -1};
int16_t pos_pair[2] = {-1, -1};

mqd_t queue_from_main;
mqd_t queue_mapping_to_main;

void printMap(){
    // We use map[y][x] as in Matlab. We print the map 180 deg flipped for readability
    for (int i = MAP_SIZE_Y-1; i>=0; i--) {
        for (int j=0; j<MAP_SIZE_X; j++){
          printf("%d", map[i][j]);
        }
        printf("\n");
    }
}

void printMap2(){
    // We use map[y][x] as in Matlab. We print the map 180 deg flipped for readability
    for (int i = MAP_SIZE_Y-1; i>=0; i--) {
        for (int j=0; j<MAP_SIZE_X; j++){
            printf("%c", printlist[map[i][j]]);
        }
        printf("\n");
    }
}

int distance_from_unmapped_tile(float ang) {
    int x, y;
    for (int dist = 0; dist < MAX_SCAN_DIST * 5; dist += TILE_SIZE) {
        y = (int)((((dist+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((dist+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);

        if (map[y][x] == UNMAPPED) {
            return dist;
        } else if (map[y][x] != EMPTY) {
            return -1;
        }

    }
}

void update_map(float ang, int dist){
    int x, y;   
    for (int i = 0; i < (dist>MAX_SCAN_DIST?MAX_SCAN_DIST:dist); i+=TILE_SIZE) {
        y = (int)((((i+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((i+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);

        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {
            // Return if a value is out of the map or we have found an obstacle there. No need to try the other values
            return;
        } else if (map[y][x] > OBSTACLE) {
            map[y][x] --; // Decrement Obstacles we cannot find anymore
        } else if (map[y][x] == UNMAPPED) {
            map[y][x] = EMPTY;
        }
    }
    if (dist < MAX_SCAN_DIST) {
        y = (int)((((dist+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((dist+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);
        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {  
            return;
        }
        if ( map[y][x] == EMPTY || map[y][x] == UNMAPPED) {
            map[y][x] = OBSTACLE;
        } else if (map[y][x] >= OBSTACLE && map[y][x] < (MAX_INCREMENTS + OBSTACLE))  {
            map[y][x]++; // Increment Obstacles we have found before
        }
    }
}

void message_handler(uint16_t command, int16_t value) {
    
    switch (command) {
        case MESSAGE_SCAN:
        break;

        case MESSAGE_SCAN_COMPLETE: {
            int16_t target_angle = -1;
            int16_t target_distance = -1;
            int16_t angle_increment = 45;

            printf("Scan complete. Searching for angle in steps of %d...\n", angle_increment);
            for (int angle = 0; angle < 360; angle += angle_increment) {
                
                int tmp = (angle + 90) % 360;
                int d = distance_from_unmapped_tile(tmp);
                if (d > 0) {
                    target_angle = tmp;
                    target_distance = d;
                    break;
                }
            }
            printf("done with for loop. angle is now %d\n", target_angle);
            if (target_angle == -1) {
                target_angle = (rand() % 8) * 45;
                target_distance = MAX_SCAN_DIST;
                printf("\tNo suitable angles found. Generated random: %d...\n", target_angle);
            } else {
                printf("\t... ok no problem, angle %d points to unmapped tile %d mm away!\n", target_angle, target_distance);
            }
            send_message(queue_mapping_to_main, MESSAGE_TARGET_DISTANCE, target_distance);
            send_message(queue_mapping_to_main, MESSAGE_TARGET_ANGLE, target_angle);
        }
        break;

        case MESSAGE_POS_X:
        case MESSAGE_POS_Y:
            // These lines are to make sure that we update both positions at the same time.
            pos_pair[command==MESSAGE_POS_X?0:1] = value;
            if (pos_pair[0] != -1 && pos_pair[1] != -1) {
                robot_x = 10 * pos_pair[0];
                robot_y = 10 * pos_pair[1];
                int x = (int)(robot_x/TILE_SIZE + 0.5);
                int y = (int)(robot_y/TILE_SIZE + 0.5);
                map[y][x] = ROBOT_POSITION;
                map[y+1][x] = EMPTY;
                map[y-1][x] = EMPTY;
                map[y][x+1] = EMPTY;
                map[y][x-1] = EMPTY;
                
                pos_pair[0] = -1;
                pos_pair[1] = -1;
            }
        break;

        case MESSAGE_ANGLE:
        case MESSAGE_SONAR:
            // These lines are to make sure that we update both angle and distance at the same time.
            data_pair[command==MESSAGE_ANGLE?0:1] = value;
            if (data_pair[0] != -1 && data_pair[1] != -1) {
                update_map((float)data_pair[0], data_pair[1]);

                data_pair[0] = -1;
                data_pair[1] = -1;
            }

        break;

        case MESSAGE_PRINT_MAP:
            printMap2();
        break;
    }
}

void *mapping_start(void* queues){
    srand(time(NULL));

    mqd_t* tmp = (mqd_t*)queues;
	queue_from_main = tmp[0];
    queue_mapping_to_main = tmp[1];

    uint16_t command;
    int16_t value;

    // hard-code the virtual fence
    for (int x = 0; x < MAP_SIZE_X; x++) {
        map[0][x] = OBSTACLE;
    }

    while(1) {
        get_message(queue_from_main, &command, &value);
        message_handler(command, value);
    }
}