#define SWEEP_MOTOR_PORT 65
#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 67
#define DEGREE_TO_LIN 2.3 // Seems to depend on battery voltage
#define COUNT_PER_ROT 360 // result of get_tacho_count_per_rot
#define WHEEL_RADIUS 2.7

int movement_init();
void *movement_start(void* queues);
