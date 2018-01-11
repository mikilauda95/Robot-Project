#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>

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
#define TILE_SIZE 50.0 //Size of each tile in mm. With decimal to ensure float division

/*uint8_t map[MAP_SIZE_Y][MAP_SIZE_X] = {UNMAPPED};*/
int robot_x = 2000; // robot start position in mm
int robot_y = 1000;
int16_t data_pair[2];
int16_t pos_pair[2] = {-1, -1};


bool found;
uint16_t dest_ang, checked;

FILE * f;

typedef struct Matrix_uint8 {
    int     rowSize;
    int     columnSize;
    uint8_t**    matrix;
} Matrix_uint8;

Matrix_uint8 createMatrix_uint8(int rowSize, int columnSize){
    Matrix_uint8 temp = {rowSize, columnSize, malloc(columnSize*sizeof(long int *))};

    if (temp.matrix == NULL) {
        /* panic */
    }

    for (int i = 0; i < rowSize; i++) {
        temp.matrix[i] = malloc(rowSize*sizeof temp.matrix[i][0]);

        if (temp.matrix[i] == NULL) {
            /* panic */
        }
    }

    temp.rowSize=rowSize;
    temp.columnSize=columnSize;

    return temp;
}

void filter_map(Matrix_uint8 map)
{
	int	i, j;
	int i2, j2;
	bool found=0;
	for (i = 1; i < map.rowSize-1; ++i) {//avoid the edges
		for (j = 1; j < map.columnSize-1; ++j) {//avoid the edges
			if (map.matrix[i][j]==OBSTACLE) {
				found=0;
				for (i2 = i-1; i2 < i+1; ++i2) {
					for (j2 = j-1; j2 < j+1; ++j2) {
						if ((j2!=j)&&(i2!=i)) {//check if points nearby have an object
							if (map.matrix[i2][j2]==OBSTACLE) {
								found=1;
							}
						}
					}
				}
				if (!found) {
					map.matrix[i][j]=MOVABLE;
				}
			}
		}
	}
}


bool check_if_exp(uint16_t angle, uint16_t distance, uint8_t** map, int robot_x, int robot_y){
	int x, y;
	x = (distance * cos(angle/180 * M_PI))/50;
	y = (distance * sin(angle/180 * M_PI))/50;
	if (map[y + robot_y][x + robot_x] == UNMAPPED){
		//not explored
		return false;
	}
	else {
		//explored already
		return true;
	}
}

uint16_t check_unexp(uint16_t angle, uint16_t distance, uint8_t** map, int robot_x, int robot_y)
{
	if (distance >= MAX_DIST) {
		if (!check_if_exp(angle, distance, map, robot_x, robot_y)){
			return angle;
		}
		else {
			return -1;
		}
	}
	else {
		return -1;
	}
}


void printMap(uint8_t** map){
    // We use map[y][x] as in Matlab. We print the map 180 deg flipped for readability
    for (int i = MAP_SIZE_Y-1; i>=0; i--) {
        for (int j=0; j<MAP_SIZE_X; j++){
            printf("%d", map[i][j]);
        }
        printf("\n");
    }
}

void update_map(float ang, int dist, uint8_t** map){
    int x, y;
    for (int i = 0; i < (dist>MAX_DIST?MAX_DIST:dist); i+=TILE_SIZE) {
        y = (int)(((i * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)(((i * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);

        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {
            // Return if a value is out of the map. No need to try the other values
            return;
        } else if (map[y][x] == UNMAPPED) {
            map[y][x] = EMPTY;
        }
    }
    if (dist < MAX_DIST) {
        y = (int)(((dist * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)(((dist * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);
        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {
            return;
        }
        map[y][x] = OBSTACLE;
        fprintf(f, "%d %d\n", x, y);
    }
    map[(int)(robot_y/TILE_SIZE + 0.5)][(int)(robot_x/TILE_SIZE +0.5)] = 7;
}

void message_handler(uint16_t command, int16_t value, Matrix_uint8 map) {
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
            printMap(map.matrix);
			send_message(movement_queue_to_main, MESSAGE_DEST_ANG, dest_ang);
            break;
    }
    if (data_pair[0] != -1 && data_pair[1] != -1) {
		//code for getting the best angle
		if (!found) {
			if((checked=check_unexp(data_pair[0], data_pair[1], map.matrix, 0, 0))>=0){//angle has been found
				dest_ang=checked;//save the destination angle
				found=1;//do not execute this anymore
			}
		}
        update_map((float)data_pair[0], data_pair[1], map.matrix);
        data_pair[0] = -1;
        data_pair[1] = -1;
    }
}

void *mapping_start(void* queues){
    mqd_t* tmp = (mqd_t*)queues;
	mqd_t queue_from_main = tmp[0];
    f = fopen("objects.txt", "w");


    uint16_t command;
    int16_t value;
	Matrix_uint8 map=createMatrix_uint8(MAP_SIZE_X, MAP_SIZE_Y);

    while(1) {
        get_message(queue_from_main, &command, &value);
        message_handler(command, value, map);
    }
}
