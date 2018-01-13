#ifndef BT_CLIENT_H_
#define BT_CLIENT_H_

#include <stdbool.h>

#include "messages.h"

#define BT_SERV_ADDR   "34:13:E8:5F:51:4E" // Bluetooth MAC address of server (run hcitool dev on server)
#define BT_TEAM_ID     1                   // team identifier placeholder
#define BT_MSG_LEN_MAX 64

// Messages from SERVER TO CLIENT as defined in server code
#define BT_MSG_ACK		0
#define BT_MSG_START	1
#define BT_MSG_STOP		2
#define BT_MSG_KICK		3

// Messages from CLIENT TO SERVER as defined in server code
#define BT_MSG_POSITION	4
#define BT_MSG_MAPDATA	5
#define BT_MSG_MAPDONE 	6

/* Initializes bluetooth connection to server with address as defined in BT_SERV_ADDR.
 */
bool bt_connect(void);

/* Blocks until a message is recived from the server. If the message was of type START,
 * return 0. If the message is of any other type, return -1. */
int bt_wait_for_start(void);

/* Main bluetooth worker. Expects an array of two message queues: from main and to main.
 * Will poll the from_main queue and for messages and parse them to the bluetooth server. 
 * 
 * TODO: listen for messages from the server and push these on the bt_rx_mq.
 * TODO: returns when server sends STOP or KICK.
 */
void* bt_client(void *queues);

#endif // !BT_CLIENT_H_