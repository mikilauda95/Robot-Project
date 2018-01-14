#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#include "mapping.h"
#include "messages.h"
#include "tuning.h"

// 1-W indicates objects with an increasing level of certainty
char *printlist = " r'XX";
char *object_list = "*ABCDEFGHI";

int8_t map[MAP_SIZE_Y][MAP_SIZE_X] = {UNMAPPED};

int robot_x = ROBOT_START_X;
int robot_y = ROBOT_START_Y;

int16_t data_pair[2] = {-1, -1};
int16_t pos_pair[2] = {-1, -1};

mqd_t queue_from_main, queue_mapping_to_main, queue_mapping_to_bt;
pthread_t map_send_thread;


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
            if (map[i][j] < UNMAPPED) {
                //printf("%d", map[i][j]<-9?9:-map[i][j]);
                printf(" ");
            } else if (map[i][j] >= UNMAPPED && map[i][j] < MAX_STRENGTH) {
                printf("%c", object_list[map[i][j]>9?9:map[i][j]]);
            } else {
                printf("%c", printlist[map[i][j] - 100]);
            }
        }
        printf("\n");
    }

}

void* map_sender(void* what){ 
	printf("map sender thread start\n");
	for (int xi = 0; xi < 80; xi++) {
		for (int yi = 0; yi < 80; yi++) {
			send_message(queue_mapping_to_bt, MESSAGE_MAP_POINT, map[yi][xi]);
		}
	}
	printf("map sender thread finished\n");
}

int distance_to_unmapped_tile(float ang) {
    int x, y;
    for (int dist = 0; dist < MAX_SCAN_DIST * 5; dist += TILE_SIZE) {
        y = (int)((((dist+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((dist+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);

        if (map[y][x] == UNMAPPED) {
            return dist;
        } else if (map[y][x] > UNMAPPED) {
            return -1;
        }

	}
}

void update_map(float ang, int dist){
    int x, y;   
    for (int i = 0; i < (dist>MAX_SCAN_DIST?MAX_SCAN_DIST:dist); i+=TILE_SIZE) {
        y = (int)((((i+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE);
        x = (int)((((i+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE);

        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {
            // Return if a value is out of the map or we have found an obstacle there. No need to try the other values
            return;
        } else if (map[y][x] < MAX_STRENGTH && map[y][x] > -MAX_STRENGTH) {
            map[y][x] --; // Decrement to indicate strength of emptyness
        }
    }
    if (dist < MAX_SCAN_DIST) {
        y = (int)((((dist+SONAR_OFFSET) * sin(ang/180 * M_PI)) + robot_y)/TILE_SIZE + 0.5);
        x = (int)((((dist+SONAR_OFFSET) * cos(ang/180 * M_PI)) + robot_x)/TILE_SIZE + 0.5);
        if (x < 0 || x >= MAP_SIZE_X || y < 0 || y >= MAP_SIZE_Y) {  
            return;
        }
        if ( map[y][x] < MAX_STRENGTH ) {
            map[y][x] ++; // Increment to indicate strengt of obstacle
        } 
    }
}

void find_optimal_target(int *tgt_ang, int *tgt_dist) {

	printf("searching for target angle/distance...\n");

	int ang_step = 1;
	int curr_ang, offset_ang;
	int d;

	// re = right edge, le = left edge
	int re_offset_ang = -1;
	int le_offset_ang = -1;
	
	uint8_t edges = 0xFF;
	const uint8_t RIGHT_EDGE_MASK = 0x0F;
	const uint8_t LEFT_EDGE_MASK = 0xF0;

	bool found_clear_path = false;

	for (offset_ang = 0; offset_ang < 360; offset_ang += ang_step) {
		
		curr_ang = (offset_ang + 90) % 360;
		d = distance_to_unmapped_tile(curr_ang);
		
		edges = (edges >> 1) & 0xFF;
		if (d == -1) {
			edges |= (1 << 7);
		} 

		if (edges == RIGHT_EDGE_MASK) {
			// right obj. edge found with 4 degrees of open space
			re_offset_ang = offset_ang;
			printf("\t found object right edge at %d degrees\n", curr_ang);
			
		} else if (edges == LEFT_EDGE_MASK) {
			// left obj. edge found with 4 degrees of open space
			le_offset_ang = offset_ang;
			printf("\t found object left edge at %d degrees\n", curr_ang);
			found_clear_path = true;
			break;
		}	

	}

	if (found_clear_path) {
		int offset_delta = re_offset_ang + 0.5 * (le_offset_ang - re_offset_ang);
		*tgt_ang = (offset_delta + 90) % 360;
		printf("\t -> choosing path at angle %d\n", *tgt_ang);
	} else {
		*tgt_ang = (rand() % 8) * 45;
		printf("\t no path found. Random angle: %d\n", *tgt_ang);
		send_message(queue_mapping_to_main, MESSAGE_NO_ROUTE_FOUND, 0);
	}

	*tgt_dist = distance_to_unmapped_tile(*tgt_ang);
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

		case MESSAGE_SEND_MAP:
		pthread_create(&map_send_thread, NULL, map_sender, NULL);
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
                            map[j][i]--;
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

}

void *mapping_start(void* queues){
	srand(time(NULL));

	mqd_t* tmp = (mqd_t*)queues;
	queue_from_main = tmp[0];
    queue_mapping_to_main = tmp[1];
    queue_mapping_to_bt = tmp[2];

	uint16_t command;
	int16_t value;

    // hard-code the virtual fence
    for (int x = 0; x < MAP_SIZE_X; x++) {
        map[0][x] = WALL;
    }

	while(1) {
		get_message(queue_from_main, &command, &value);
		message_handler(command, value);
	}
}