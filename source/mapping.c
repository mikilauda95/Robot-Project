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

int robot_x = ROBOT_START_X;
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

int distance_to_unmapped_tile(float ang) {
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

// todo: add distance to object so we can drive to farthest covrner

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

void find_optimal_target(int *tgt_ang, int *tgt_dist) {

	printf("searching for target angle/distance...\n");

	const int obj_count_max = 10;
	int obj_cnt = obj_count_max;

	int d, maxd, maxd_ang, mind;
	maxd = 0;
	mind = MAX_SCAN_DIST;

	// re = right edge, le = left edge
	int re_ang, le_ang;
	bool has_found_right_edge = false;
	bool has_found_clear_path = false;

	for (int curr_ang = 0; curr_ang < 360; curr_ang++) {

		if (has_found_right_edge && curr_ang == 180) {
			// if we search >90 degrees after finding a right edge, stop.
			printf("\t >180 deg free after right edge. stop\n");
			has_found_clear_path = true;
			le_ang = curr_ang;
			break;
		}
		
		d = distance_to_unmapped_tile(curr_ang);
		if (d > maxd) {
			maxd = d;
			maxd_ang = curr_ang;
		}
		if (d < mind && d > 0) {
			mind = d;
		}

		if (d > 0 && obj_cnt > 0){
			--obj_cnt;
			if (obj_cnt == obj_count_max / 2){
				re_ang = curr_ang - obj_count_max / 2 + 1;
				printf("\t found object right edge at %d degrees\n", re_ang);
				has_found_right_edge = true;
			}

		} else if (d < 0 && obj_cnt < obj_count_max){
			++obj_cnt;
			if (obj_cnt == obj_count_max / 2 && has_found_right_edge){
				le_ang = curr_ang - obj_count_max / 2 + 1;
				printf("\t found object left edge at %d degrees\n", le_ang);
				has_found_clear_path = true;
				break;
			}
		}

	}

	if (has_found_clear_path) {
		le_ang += 2000 / mind;
		printf("\t shortest distance was %d -> setting le to %d degrees\n", mind, le_ang);
		int delta = le_ang > re_ang ? (le_ang - re_ang) : (360 - (le_ang - re_ang));
		*tgt_ang = (int)(re_ang + 0.5 * (le_ang - re_ang)) % 360;
		printf("\t -> choosing path at angle %d\n", *tgt_ang);
		*tgt_dist = distance_to_unmapped_tile(*tgt_ang);
	} else {
		*tgt_ang = maxd_ang;
		*tgt_dist = maxd / 2;
		printf("\t no path found. moving halfway across room: %d deg\n", *tgt_ang);
		printf("\t however maxd is %d at % degrees \n", maxd, maxd_ang);
	}

	if (*tgt_dist == -1) {
		// something went wrong and we chose a path pointing into a wall.. not good.
		// try to drive as far as possible then.
		*tgt_ang = maxd_ang;
		*tgt_dist = maxd;
	}

	printf("\t -> target distance: %d\n", *tgt_dist);

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
                robot_x = 10 * pos_pair[0];
                robot_y = 10 * pos_pair[1];
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