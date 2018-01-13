//TODO: transform the update function so that it takes as arguments x and y and just update the map with the values corrected
//
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "messages.h"

#define UNMAPPED 0
#define EMPTY 1
#define ROBOT_POSITION 2
#define MOVABLE 3
#define VIRTUAL_WALL 4
#define WALL 5
#define OBJECT_DROPPED 6
#define VER_WALL 7
#define HOR_WALL 8
#define OBSTACLE 10

#define MAX_INCREMENTS 31
// 1-W indicates objects with an increasing level of certainty
char *printlist = "* r'?X+|_?123456789ABCDEFGHIJKLMNOPQRSTUVW";

#define HOR_SIZE 24
#define VER_SIZE 40

#define OPTION 0

#define MAP_SIZE_X 80
#define MAP_SIZE_Y 80
#define MAX_DIST 400 // Max distance in mm
#define TILE_SIZE 50.0 // Size of each tile in mm. With decimal to ensure float division
#define SONAR_OFFSET 100 // Distance from rotation axis to the sonar in mm

#define MAX_DELTA_ANG 4

int8_t map[MAP_SIZE_Y][MAP_SIZE_X] = {UNMAPPED};

int robot_x; 
int robot_y;

int16_t data_pair[2] = {-1, -1};
int16_t pos_pair[2] = {-1, -1};
//
//USED FOR THE RECALIBRATION
//
int min_distance_x=3000;//set to a very large number
int min_distance_y=3000;//set to a very large number
int correction_x=0;
int correction_y=0;
uint8_t tmp_map[MAP_SIZE_Y][MAP_SIZE_X];

mqd_t queue_from_main;
mqd_t queue_mapping_to_main;
FILE * f;

void filter_map(int option){
	int i;
	if (option==0) {
		//mapping the horizontal lines
		for (i = 1; i < HOR_SIZE-1 ; ++i) {
			/*printf("debug\n");*/
			map[1][i]=EMPTY;
			map[VER_SIZE+1][i]=EMPTY;
			map[VER_SIZE-1][i]=EMPTY;
		}
		//mapping the vertical lines
		for (i = 1; i < VER_SIZE-1 ; ++i) {
			/*printf("debug\n");*/
			map[i][HOR_SIZE+1]=EMPTY;
			map[i][HOR_SIZE-1]=EMPTY;
			map[i][1]=EMPTY;
		}
	}
}


void initialize_map(int option){
	if (option==0) {//arena map hardcoding
		printf("In initialize\n");
		//mapping the horizontal lines
		int	i;
		for (i = 0; i < HOR_SIZE ; i++) {
			/*printf("debug\n");*/
			map[0][i]=HOR_WALL;
			map[VER_SIZE][i]=HOR_WALL;
			printf("HOR wall\n");
		}
		//mapping the vertical lines
		for (i = 0; i < VER_SIZE ; i++) {
			/*printf("debug\n");*/
			map[i][HOR_SIZE]=VER_WALL;
			map[i][0]=VER_WALL;
			printf("VER wall\n");
		}
		printf("finished here\n");
	}
}


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
	for (int dist = 0; dist < MAX_DIST * 5; dist += TILE_SIZE) {
		y = (int)((((dist+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
		x = (int)((((dist+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);

		if (map[y][x] == UNMAPPED) {
			return dist;
		} else if (map[y][x] != EMPTY) {
			return -1;
		}

	}
}

/*void readjust_to_walls(int x, int y, int distance){*/
    /*int i;*/
    /*//calibrate to vertical walls*/
    /*for (i = 0; i < 4; ++i) {*/
        /*if (map[y][x+2-i]==VER_WALL) {*/
            /*if (distance<min_distance_x) {*/
                /*printf("readjusted with vertical wall when reading %d %d and it was %d close \n", x, y, 2-i);*/
                /*min_distance_x=distance;*/
                /*correction_x=2-i;*/
            /*}*/
        /*} */
    /*}*/
    /*//calibrate to horizontal walls*/
    /*for (i = 0; i < 4; ++i) {*/
        /*if (map[y+2-i][x]==HOR_WALL) {*/
            /*if (distance<min_distance_y) {*/
                /*printf("readjusted with horizontal wall when reading %d %d and it was %d close \n", x, y, 2-i);*/
                /*min_distance_y=distance;*/
                /*correction_y=2-i;*/
            /*}*/
        /*} */
    /*}*/
/*}*/


void readjust_to_walls(int x, int y){
    int i;
    //calibrate to vertical walls
    for (i = -1; i <= 1; ++i) {
        if (map[y][x+2-i]==VER_WALL) {
            printf("readjusted with vertical wall when reading %d %d and it was %d close \n", x, y, i);
            printf("X coordinate before readjustement= %d\n", robot_x);
            robot_x +=(i)*50;
            printf("X coordinate after readjustement= %d\n", robot_x);
        }
    } 
    //calibrate to horizontal walls
    for (i = -1; i <= 1; ++i) {
        if (map[y+i][x]==HOR_WALL) {
            printf("readjusted with horizontal wall when reading %d %d and it was %d close \n", x, y, i);
            printf("Y coordinate before readjustement= %d\n", robot_y);
            robot_y+=(i)*50;
            printf("Y coordinate after readjustement= %d\n", robot_y);
        }
    }
}


void update_map(float ang, int dist){
    int x, y;   
    for (int i = 0; i < (dist>MAX_DIST?MAX_DIST:dist); i+=TILE_SIZE) {
        y = (int)((((i+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((i+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);
        //we add 0.5 in order to have the right rounding

        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {
            // Return if a value is out of the map or we have found an obstacle there. No need to try the other values
            return;
        } else if (map[y][x] > OBSTACLE) {
            map[y][x] --; // Decrement Obstacles we cannot find anymore
        } else if (map[y][x] == UNMAPPED) {
            map[y][x] = EMPTY;
        }
    }
    if (dist < MAX_DIST) {
        //only recalibrate when it is almost perpendicular to the wall for the first 5 centimeters
        if (abs(ang-90)<MAX_DELTA_ANG||abs(ang-180)<MAX_DELTA_ANG||abs(ang-270)<MAX_DELTA_ANG||ang<(MAX_DELTA_ANG/2)||(360-ang<MAX_DELTA_ANG/2)) {
            readjust_to_walls(x,y);
        }
        y = (int)((((dist+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((dist+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);
        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {  
            return;
        }
        if (map[y][x]!=OBJECT_DROPPED) {
            if ( map[y][x] == EMPTY || map[y][x] == UNMAPPED) {
                map[y][x] = OBSTACLE;
            } else if (map[y][x] >= OBSTACLE && map[y][x] < (MAX_INCREMENTS + OBSTACLE))  {
                map[y][x]++; // Increment Obstacles we have found before
            }
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
                target_distance = MAX_DIST;
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
            filter_map(OPTION);
            printMap2();
        break;
        case MESSAGE_UPDATE_OBJECT:
            printf("NOW I MAP THE OBJECT\n");
            map[robot_y][robot_x]=OBJECT_DROPPED;
        break;
    }
}

void *mapping_start(void* queues){
	srand(time(NULL));

	mqd_t* tmp = (mqd_t*)queues;
	queue_from_main = tmp[0];
	queue_mapping_to_main = tmp[1];
	initialize_map(OPTION);
	printf("initial map\n");
	printMap2();
	f = fopen("objects.txt", "w");

	uint16_t command;
	int16_t value;

	// hard-code the virtual fence
	/*for (int x = 0; x < MAP_SIZE_X; x++) {*/
	/*map[0][x] = OBSTACLE;*/
	/*}*/

	while(1) {
		get_message(queue_from_main, &command, &value);
		message_handler(command, value);
	}
}
