#include <mqueue.h>

// My idea here is something like send (turn*2048 + deg)  and decode at the other end. 
// This will help us send command and value in the same int
#define STOP 0
#define TURN 1
#define SET_SPEED 2
#define PIROUETTE 3

mqd_t init_queue (char* mq_name, int open_flags);
int get_integer_from_mq(mqd_t mqd);
void put_integer_in_mq(mqd_t mqd, int data);