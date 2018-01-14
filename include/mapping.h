#define MAX_STRENGTH 100
#define UNMAPPED 0
#define EMPTY 1
#define ROBOT_POSITION 2
#define DROPPED_OBJECT 3
#define WALL 5
#define HOR_WALL 6
#define VER_WALL 7
#define OBSTACLE 10

#define ARENA 0
#define NO_ARENA 1
#define MAP_SIZE_X 80
#define MAP_SIZE_Y 80
#define TILE_SIZE 50.0 // Size of each tile in mm. With decimal to ensure float division
#define SONAR_OFFSET 100 // Distance from rotation axis to the sonar in mm

#define IS_EMPTY(tile) ((tile == EMPTY) || (tile == ROBOT_POSITION))
#define IS_OBJECT(tile) ((tile >= OBSTACLE) || (tile == WALL))

void *mapping_start(void* queues);
