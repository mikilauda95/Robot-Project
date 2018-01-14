#define ARENA 0
#define ROBOT_START_X 600 // Robot start position in mm
#define ROBOT_START_Y 600
#define START_HEADING 90 // Robot start heading in degrees
#define MAX_SCAN_DIST 500 // Max distance in mm

#define MAP_SIZE_X 80
#define MAP_SIZE_Y 80
#define TILE_SIZE 50.0 // Size of each tile in mm. With decimal to ensure float division

#define FORWARD_SPEED 350 // Wheel speed when running forward. Max is 1050
#define TURN_SPEED 250 
#define SCAN_SPEED 100
#define SWEEP_SPEED 350 // Speed of the motor that sweeps the US-sensor
#define DEGREE_TO_LIN 2.5 // Factor for use in turn_degrees. Seems to depend on battery voltage
#define SWEEP_MOTOR_PORT 65
#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 68
#define ARM_MOTOR_PORT 67
#define OBJECT_OFFSET 200

#define SENSOR_UPDATE_TIME 40 // Time delay between subsequent sensor reads in ms

#define SCANS_BEFORE_RELEASE 2

#define CURRENT_ENVIRONMENT 0 // 0: small arena, 1: big arena

#define SENSOR_UPDATE_TIME 40 // Time delay between subsequent sensor reads in ms
