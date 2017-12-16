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

/* to add an integer to the message queue */
void send_message(mqd_t mq, uint16_t command, int16_t value) {
    int32_t message = ((command<<16) & 0xFFFF0000) | (value & 0xFFFF); //put the command and the value into the same int.

    int status = mq_send (mq, (char *) &message, sizeof(int32_t), 1); //sizeof for readability. 4 bytes
    if (status == -1)
      printf ("mq_send failure\n");

}

 /* to get an integer from message queue */
int get_message (mqd_t mq, uint16_t *command, int16_t *value) {
    ssize_t num_bytes_received = 0;
    int32_t data=0;

    num_bytes_received = mq_receive(mq, (char *) &data, sizeof(int32_t), NULL);
    //decode the message
    *value = data & 0x0000ffff;
    *command = data >> 16;
    
    return num_bytes_received;
}
