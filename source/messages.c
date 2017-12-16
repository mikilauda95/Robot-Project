#include <stdio.h>
#include <stdlib.h>
#include "messages.h"

#define MAX_MSG 15
#define OVER 37
#define MQ_SIZE 10
#define PMODE 0666

/* to open the message queue */
mqd_t init_queue (char* mq_name, int open_flags) {
    struct mq_attr attr;
    // fill attributes of the mq
    attr.mq_maxmsg = MQ_SIZE;
    attr.mq_msgsize = sizeof (int);
    attr.mq_flags = 0;
    
    mqd_t mqfd =  mq_open(mq_name, open_flags, PMODE, &attr);

    if (mqfd == (mqd_t)-1) {
        perror("Mq opening failed");
        exit(-1);
    }
    return mqfd;
}

void send_message(mqd_t mq, uint16_t command, int value) {
    uint16_t send_val;

    if (value>=0) {
        send_val = (uint16_t)value;
    } else {
        send_val = (uint16_t)-value;
        send_val = send_val | 0x8000; // Set the highest bit to 1 to indicate negative number. 
        // TODO check that highest bit was not already 1. Negative numbers cannot be as big as the positive.
    }
    uint32_t message = (command<<16) | send_val; //put the command and the value into the same int.

    int status = mq_send (mq, (char *) &message, sizeof(uint32_t), 1); //sizeof for readability. 4 bytes
    if (status == -1)
      perror ("mq_send failure");
}


int get_message (mqd_t mq, uint16_t *command, int *value) {
    ssize_t num_bytes_received = 0;
    uint32_t data=0;
    int temp;

    num_bytes_received = mq_receive(mq, (char *) &data, sizeof(uint32_t), NULL);
    //decode the message
    temp = data & 0x0000ffff;
    // Check for negative number
    *value = ((temp&0x8000) >0)?(int)-temp:(int)temp
    *command = data >> 16;
    return num_bytes_received;
}
