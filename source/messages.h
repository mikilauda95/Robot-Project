#include <mqueue.h>
#include <stdint.h>

// My idea here is something like send((MESSAGE_TURN << 11)  | deg))  and decode at the other end. 
// This will help us send command and value in the same int
#define MSG_MOV_STOP 0
#define MSG_MOV_TURN 1
#define MSG_MOV_SET_SPEED 2
#define MSG_MOV_PIROUETTE 3
#define MSG_MOV_RUN_FORWARD 4

#define MSG_SENS_COMPASS 10
#define MSG_SENS_SONAR 11

mqd_t init_queue (char *mq_name, int open_flags);
void send_message(mqd_t mq, uint16_t command, uint16_t value);
void get_message(mqd_t mq, uint16_t *command, uint16_t *value);