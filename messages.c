#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>

#define MAX_MSG 15
#define OVER 37
#define MQ_NAME "/movement"
#define MQ_SIZE 10
#define PMODE 0666

mqd_t mqfd_movement;
/* to open the message queue */
void init_queue (int open_flags) {
    struct mq_attr attr;
    // fill attributes of the mq
    attr.mq_maxmsg = MQ_SIZE;
    attr.mq_msgsize = sizeof (int);
    attr.mq_flags = 0;
    
    mqfd_movement = mq_open(MQ_NAME, open_flags, PMODE, &attr);

    if (mqfd_movement == (mqd_t)-1) {
        perror("Mq opening failed");
        exit(-1);
    }
}

/* to add an integer to the message queue */
void put_integer_in_mq (int data) {
  int status;
   //sends message
  status = mq_send (mqfd_movement, (char *) &data, sizeof (int), 1);
  if (status == -1)
    perror ("mq_send failure");
}

 /* to get an integer from message queue */
int get_integer_from_mq () {
  ssize_t num_bytes_received = 0;
  int data=0;
   //receive an int from mq
  num_bytes_received = mq_receive (mqfd_movement, (char *) &data, sizeof (int), NULL);
  if (num_bytes_received == -1)
    perror ("mq_receive failure");
  return (data);
}

int messages_init() {
    init_queue(O_CREAT | O_RDWR);
    return 0;
}
