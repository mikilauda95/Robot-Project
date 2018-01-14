#define COUNT_PER_ROT 360 // result of get_tacho_count_per_rot
#define WHEEL_RADIUS 27 // in mm

int movement_init();
void *movement_start(void* queues);
