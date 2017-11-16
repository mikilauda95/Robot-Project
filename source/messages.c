#include <stdio.h>
#include <stdlib.h>
#include "messages.h"

#define MAX_MSG 15
#define OVER 37
#define MQ_NAME "/movement"
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

/* to add an integer to the message queue */
void put_integer_in_mq(mqd_t mq, int command, int value) {
    int message = (command<<11) | value; //put the command and the value into the same int.
    int status;
    //send message
    status = mq_send (mq, (char *) &message, sizeof (int), 1);
    if (status == -1)
      perror ("mq_send failure");
}

 /* to get an integer from message queue */
void get_integer_from_mq (mqd_t mq, int *command, int *value) {
    ssize_t num_bytes_received = 0;
    int data=0;
    //receive an int from mq

    num_bytes_received = mq_receive(mq, (char *) &data, sizeof (int), NULL);
    if (num_bytes_received == -1)
      perror ("mq_receive failure");
    //decode the message
    *value = data & 0x7ff;
    *command = data >> 11;
}
