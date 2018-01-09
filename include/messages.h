#include <mqueue.h>
#include <stdint.h>

// My idea here is something like send((MESSAGE_TURN << 11)  | deg))  and decode at the other end. 
// This will help us send command and value in the same int
#define MESSAGE_STOP 0
#define MESSAGE_TURN_DEGREES 1
#define MESSAGE_SET_SPEED 2
#define MESSAGE_FORWARD_COMPLETE 3
#define MESSAGE_FORWARD 4
#define MESSAGE_SCAN 5
#define MESSAGE_TURN_COMPLETE 6
#define MESSAGE_SCAN_COMPLETE 7
#define MESSAGE_HEADING 8
#define MESSAGE_SCAN_STARTED 9

#define MESSAGE_ANGLE 10
#define MESSAGE_SONAR 11

#define MESSAGE_POS_X 20
#define MESSAGE_POS_Y 21

#define MESSAGE_PRINT_MAP 30
#define MESSAGE_DEST_ANGLE 31

#define MESSAGE_TARGET_ANGLE 40


mqd_t init_queue (char *mq_name, int open_flags);
void send_message(mqd_t mq, uint16_t command, int16_t value);
int get_message(mqd_t mq, uint16_t *command, int16_t *value);