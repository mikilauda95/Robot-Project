#define MAX_STRENGTH 100
#define UNMAPPED 0
#define ROBOT_POSITION 101
#define MOVABLE 102
#define VIRTUAL_WALL 103
#define WALL 104

#define MAP_SIZE_X 80
#define MAP_SIZE_Y 80
#define TILE_SIZE 50.0 // Size of each tile in mm. With decimal to ensure float division
#define SONAR_OFFSET 100 // Distance from rotation axis to the sonar in mm

void *mapping_start(void* queues);
