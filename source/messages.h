#include <mqueue.h>
#include <stdint.h>

// Movement commands
#define MSG_MOV_STOP 0
#define MSG_MOV_TURN 1
#define MSG_MOV_SET_SPEED 2
#define MSG_MOV_PIROUETTE 3
#define MSG_MOV_RUN_FORWARD 4

// Movement status update
#define MSG_MOV_TURN_COMPLETE 5
#define MSG_MOV_SCAN_COMPLETE 6

// Sensor value types
#define MSG_SENS_COMPASS 10
#define MSG_SENS_SONAR 11
#define MSG_SENS_COLOR 12

mqd_t init_queue (char *mq_name, int open_flags);
void send_message(mqd_t mq, uint16_t command, uint16_t value);
void get_message(mqd_t mq, uint16_t *command, uint16_t *value);