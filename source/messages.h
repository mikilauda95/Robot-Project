#include <mqueue.h>
#include <stdint.h>

// My idea here is something like send((MESSAGE_TURN << 11)  | deg))  and decode at the other end. 
// This will help us send command and value in the same int
#define MESSAGE_STOP 0
#define MESSAGE_TURN 1
#define MESSAGE_SET_SPEED 2
#define MESSAGE_PIROUETTE 3
#define MESSAGE_FORWARD 4
#define MESSAGE_COMPASS 10
#define MESSAGE_SONAR 11
#define MESSAGE_SCANN 12
#define MESSAGE_AVGCOMPASS 13
#define MESSAGE_END_SCANN 14
#define MESSAGE_CONTINUE 15

mqd_t init_queue (char *mq_name, int open_flags);
void send_message(mqd_t mq, uint16_t command, uint16_t value);
void get_message(mqd_t mq, uint16_t *command, uint16_t *value);
