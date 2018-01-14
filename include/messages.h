#include <mqueue.h>
#include <stdint.h>

#define MAX_MSG 15
#define OVER 37
#define MQ_SIZE 10
#define PMODE 0666

#define MESSAGE_STOP 0
#define MESSAGE_TURN_DEGREES 1
#define MESSAGE_REACHED_DEST 2
#define MESSAGE_FORWARD 3
#define MESSAGE_SCAN 4
#define MESSAGE_TURN_COMPLETE 5
#define MESSAGE_SCAN_COMPLETE 6
#define MESSAGE_CALCULATE_NEXT_MOVE 7
#define MESSAGE_HEADING 8
#define MESSAGE_SCAN_STARTED 9

#define MESSAGE_ANGLE 10
#define MESSAGE_SONAR 11

#define MESSAGE_POS_X 20
#define MESSAGE_POS_Y 21

#define MESSAGE_PRINT_MAP 30
#define MESSAGE_DEST_ANGLE 31
#define MESSAGE_TARGET_ANGLE 32
#define MESSAGE_TARGET_DISTANCE 33
#define MESSAGE_NO_OPTIMAL_TARGET 34

#define MESSAGE_MAP_X_DIM 41
#define MESSAGE_MAP_Y_DIM 42
#define MESSAGE_MAP_POINT 43

mqd_t init_queue (char *mq_name, int open_flags);
void send_message(mqd_t mq, uint16_t command, int16_t value);
int get_message(mqd_t mq, uint16_t *command, int16_t *value);