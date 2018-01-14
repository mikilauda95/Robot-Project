
#define MAX_STRENGTH 100
#define UNMAPPED 0
#define ROBOT_POSITION 101
#define MOVABLE 102
#define VIRTUAL_WALL 103
#define WALL 104
#define OBJECT_DROPPED 105
#define VER_WALL 106
#define HOR_WALL 107
#define CERTAIN_EMPTY 108

#define MAP_SIZE_X 80
#define MAP_SIZE_Y 80
#define TILE_SIZE 50.0 // Size of each tile in mm. With decimal to ensure float division
#define SONAR_OFFSET 100 // Distance from rotation axis to the sonar in mm

#define ARENA 0
#define NO_ARENA 1


void *mapping_start(void* queues);
