#define UNMAPPED 0
#define EMPTY 1
#define ROBOT_POSITION 2
#define DROPPED_OBJECT 3
#define WALL 5
#define OBSTACLE 10

#define SONAR_OFFSET 100 // Distance from rotation axis to the sonar in mm

void *mapping_start(void* queues);
