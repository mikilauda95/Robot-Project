#define UNMAPPED 0
#define EMPTY 1
#define ROBOT_POSITION 2
#define MOVABLE 3
#define VIRTUAL_WALL 4
#define WALL 5
#define OBSTACLE 10

#define MAP_SIZE_X 80
#define MAP_SIZE_Y 80
#define TILE_SIZE 50.0 // Size of each tile in mm. With decimal to ensure float division
#define SONAR_OFFSET 100 // Distance from rotation axis to the sonar in mm

void *mapping_start(void* queues);
