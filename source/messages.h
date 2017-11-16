#include <mqueue.h>

// My idea here is something like send (turn*2048 + deg)  and decode at the other end. 
// This will help us send command and value in the same int
#define MESSAGE_STOP 0
#define MESSAGE_TURN 1
#define MESSAGE_SET_SPEED 2
#define MESSAGE_PIROUETTE 3
#define MESSAGE_FORWARD 4

#define MESSAGE_COMPASS 10
#define MESSAGE_SONAR 11

mqd_t init_queue (char *mq_name, int open_flags);
void get_integer_from_mq(mqd_t mq, int *command, int *value);
void put_integer_in_mq(mqd_t mq, int command, int value);