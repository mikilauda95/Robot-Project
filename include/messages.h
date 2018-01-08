#include <mqueue.h>
#include <stdint.h>

// My idea here is something like send((MESSAGE_TURN << 11)  | deg))  and decode at the other end. 
// This will help us send command and value in the same int
#define MESSAGE_STOP 0
#define MESSAGE_TURN_DEGREES 1
#define MESSAGE_SET_SPEED 2
#define MESSAGE_PIROUETTE 3
#define MESSAGE_FORWARD 4
#define MESSAGE_TURN_COMPLETE 5
#define MESSAGE_HEADING 6

#define MESSAGE_ANGLE 10
#define MESSAGE_SONAR 11

#define MESSAGE_POS_X 20
#define MESSAGE_POS_Y 21

#define MESSAGE_MAP_START 30
#define MESSAGE_MAP_X_DIM 31
#define MESSAGE_MAP_Y_DIM 32
#define MESSAGE_MAP_POINT 33

mqd_t init_queue (char *mq_name, int open_flags);
void send_message(mqd_t mq, uint16_t command, int16_t value);
int get_message(mqd_t mq, uint16_t *command, int16_t *value);