#include <mqueue.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_MSG 15
#define OVER 37
#define MQ_NAME "/mqname" /* Message queue name: To be modified. The name */
                          /* must start with a "/" */
#define MQ_SIZE 10
#define PMODE 0666


/* to open the message queue */
void init_queue (mqd_t *mq_desc, int open_flags) {
  struct mq_attr attr;
  
  // fill attributes of the mq
  attr.mq_maxmsg = MQ_SIZE;
  attr.mq_msgsize = sizeof (int);
  attr.mq_flags = 0;

  // open the mq
  //*mq_desc = mq_open (MQ_NAME, open_flags);
  *mq_desc = mq_open (MQ_NAME, open_flags, PMODE, &attr);
  if (*mq_desc == (mqd_t)-1) {
    perror("Mq opening failed");
    exit(-1);
  }
}


/* to add an integer to the message queue */
void put_integer_in_mq (mqd_t mq_desc, int data) {
  int status;
  
  //sends message
  status = mq_send (mq_desc, (char *) &data, sizeof (int), 1);
  if (status == -1)
    perror ("mq_send failure");
}


/* to get an integer from message queue */
int get_integer_from_mq (mqd_t mq_desc) {
  ssize_t num_bytes_received = 0;
  int data=0;

  //receive an int from mq
  num_bytes_received = mq_receive (mq_desc, (char *) &data, sizeof (int), NULL);
  if (num_bytes_received == -1)
    perror ("mq_receive failure");
  return (data);
}


/* code of a sending process */
void sender ()
{
  int n;
  mqd_t mqfd;

  printf("I am in sender\n");
  init_queue (&mqfd, O_CREAT | O_WRONLY);
  for (n = 0; n < MAX_MSG; n++) {
      printf ("%d ?--->\n", n);
      put_integer_in_mq (mqfd, n);
    }
  put_integer_in_mq (mqfd, OVER);
  
  /* close queue */
  mq_close (mqfd);
  return ;
}

/* code of receiving process */
void receiver ()
{
  int d;
  mqd_t mqfd;

  printf("I am in receiver\n");
  init_queue (&mqfd, O_CREAT | O_RDONLY);
  while (1) {
      d = get_integer_from_mq (mqfd);
      if (d == OVER)
	break;
      printf ("---> %d\n", d);
    }
  /* Close queue */
  mq_close(mqfd);

  return ;
}


int main(void) {
  int pid;

  // Create two processes: one for sending data, and another one for receiving data
  
  pid = fork();

  if (pid != 0) { /* Parent */
	receiver();
 	// delete the message queue
  	mq_unlink(MQ_NAME);

  } else { /* Child */
	sender();
  }
  
}