
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
char *printlist = "* r+?X????123456789ABCDEFGHIJKLMNOPQRSTUVW";

int8_t map[MAP_SIZE_Y][MAP_SIZE_X] = {UNMAPPED};

int robot_x = ROBOT_START_X;
int robot_y = ROBOT_START_Y;

int obj_x, obj_y;

int16_t data_pair[2] = {-1, -1};
int16_t pos_pair[2] = {-1, -1};
int16_t drop_pair[2] = {-1, -1};

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

/*
	returns the distance to the closest nonempty tile given an angle from Marvin's
	x-y-position. Places the tile value in the tile pointer.
	
	Author: Ola Nordmann
*/
int distance_to_nonempty_tile(float ang, int8_t *tile) {

    int x, y;
	int8_t tmp;
	int search_dist = (MAP_SIZE_X > MAP_SIZE_Y) ? MAP_SIZE_X * TILE_SIZE : MAP_SIZE_Y * TILE_SIZE;
    
	for (int dist = 0; dist < search_dist; dist += TILE_SIZE) {
        y = (int)((((dist+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((dist+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);

		tmp = map[y][x];
		if (!IS_EMPTY(tmp)) {
			*tile = tmp;
			return dist;
		}

	}
}

void update_map(float ang, int dist){
    int x, y;   
    for (int i = 0; i < (dist>MAX_SCAN_DIST?MAX_SCAN_DIST:dist); i+=TILE_SIZE) {
        y = (int)((((i+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((i+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);

        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {
            // Return if a value is out of the map or we have found an obstacle there. 
			// No need to try the other values
            return;
        } else if (map[y][x] > OBSTACLE) {
            // map[y][x] --; // Decrement Obstacles we cannot find anymore
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

/*
	finds the optimal direction and length of travel for Marvin based on Marvin's current
	position. This is done by searching for the first open passage that is at least 5 degrees 
	wide. The search starts at 0 degrees (along the x axis of the map).
	
	Author:
*/
void find_optimal_target(int *best_ang, int *best_dist) {

	const int obj_count_max = 20;
	int obj_cnt = obj_count_max;
	int curr_ang;

	int curr_dist, min_dist, max_dist, maxd_ang;
	min_dist = MAX_SCAN_DIST;
	max_dist = 0;

	int8_t tile;

	// re = right edge, le = left edge
	int re_ang, le_ang;
	bool has_found_right_edge = false;
	bool has_found_clear_path = false;

	for (int curr_ang = 45; curr_ang < (360+180); curr_ang++) {

		if (has_found_right_edge && curr_ang >= (re_ang + 90)) {
			// if we search >90 degrees after finding a right edge, stop.
			printf("\t >90 deg free after right edge. stop\n");
			has_found_clear_path = true;
			le_ang = curr_ang;
			break;
		}
		
		curr_dist = distance_to_nonempty_tile(curr_ang, &tile);

		if (IS_OBJECT(tile)) {
			if (curr_dist > max_dist) {
				max_dist = curr_dist;
				maxd_ang = curr_ang;
			} else if (curr_dist < min_dist) {
				min_dist = curr_dist;
			}
		}

		if (tile == UNMAPPED && obj_cnt > 0) {
			--obj_cnt;
			if (obj_cnt == obj_count_max / 2){
				obj_cnt = 0;
				re_ang = curr_ang - obj_count_max / 2 + 1;
				printf("\t found object right edge at %d degrees\n", re_ang);
				has_found_right_edge = true;
			}
		}

		if (IS_OBJECT(tile) && obj_cnt < obj_count_max) {
			++obj_cnt;
			if (obj_cnt == (obj_count_max / 2) && has_found_right_edge){
				le_ang = curr_ang - obj_count_max / 2 + 1;
				printf("\t found object left edge at %d degrees\n", le_ang);
				has_found_clear_path = true;
				break;
			}
		}

	} // end for angle

	if (has_found_clear_path) {
		*best_ang = (int)(re_ang + 0.5 * (le_ang - re_ang));
		printf("\t target angle %d\n", *best_ang);
		*best_dist = distance_to_nonempty_tile(*best_ang, &tile);
	} else {
		*best_ang = maxd_ang;
		*best_dist = max_dist / 2;
		printf("\t no path found. moving halfway across room: %d deg %d cm\n", *best_ang, *best_dist);
	}

	if (*best_dist == -1) {
		// something went wrong and we chose a path pointing into a wall.. not good.
		// try to drive as far as possible then.
		printf("\t FAIL: this path points into wall! driving across room: %d deg %d cm\n", *best_ang, *best_dist);		
		*best_ang = maxd_ang;
		*best_dist = max_dist / 2;
	}

	printf("\t target distance: %d\n", *best_dist);
	*best_ang %= 360;
}

void message_handler(uint16_t command, int16_t value) {
	
	switch (command) {
		case MESSAGE_SCAN:
		break;

		case MESSAGE_SCAN_COMPLETE: {
			int target_angle, target_distance;
			printf("SCAN complete. Searching for optimal target\n");
			find_optimal_target(&target_angle, &target_distance);
			printf("Finished.\n");
			
			send_message(queue_mapping_to_main, MESSAGE_TARGET_DISTANCE, target_distance);
			send_message(queue_mapping_to_main, MESSAGE_TARGET_ANGLE, target_angle);

		}
		break;

		case MESSAGE_POS_X:
		case MESSAGE_POS_Y:
            // These lines are to make sure that we update both positions at the same time.
            pos_pair[command==MESSAGE_POS_X?0:1] = value;
            if (pos_pair[0] != -1 && pos_pair[1] != -1) {
                robot_x = pos_pair[0];
                robot_y = pos_pair[1];
                int x = (int)(robot_x/TILE_SIZE + 0.5);
                int y = (int)(robot_y/TILE_SIZE + 0.5);
                map[y][x] = ROBOT_POSITION;
                for (int i = x-1; i < x+1; i++) {
                    for (int j = y-1; j < y+1; j++) {
                        if (map[j][i] == UNMAPPED) {
                            map[j][i] = EMPTY;
                        }
                    }
                }
                
                pos_pair[0] = -1;
                pos_pair[1] = -1;
            }
		break;

        case MESSAGE_ANGLE:
        case MESSAGE_SONAR:
            // These lines are to make sure that we update both angle and distance at the same time.
            data_pair[command==MESSAGE_ANGLE?0:1] = value;
            if (data_pair[0] != -1 && data_pair[1] != -1) {
                // We have received the angle in centidegrees, therefore we divide by 10.
                update_map(data_pair[0]/10.0, data_pair[1]);
                data_pair[0] = -1;
                data_pair[1] = -1;
            }

		break;

		case MESSAGE_PRINT_MAP:
			printMap2();
		break;
	}

        case MESSAGE_PRINT_MAP:
            printMap2();
        break;
        case MESSAGE_DROP_X:
        case MESSAGE_DROP_Y:
            drop_pair[command==MESSAGE_DROP_X?0:1] = value;
            if (drop_pair[0] != -1 && drop_pair[1] != -1) {
                obj_x=(int)(drop_pair[0]/TILE_SIZE);
                obj_y=(int)(drop_pair[1]/TILE_SIZE);
                printf("%d, %d --> %d, %d \n", obj_x, obj_y, drop_pair[0], drop_pair[1]);

                map[obj_y][obj_x]=DROPPED_OBJECT;
                drop_pair[0] = -1;
                drop_pair[1] = -1;
            }
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

    for (int x = 0; x < MAP_SIZE_X; x++) {
        map[0][x] = WALL;
    }

	while(1) {
		get_message(queue_from_main, &command, &value);
		message_handler(command, value);
	}
}