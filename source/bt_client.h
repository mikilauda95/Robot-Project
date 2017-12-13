#ifndef BT_CLIENT_H_
#define BT_CLIENT_H_

#include "messages.h"

#define BT_SERV_ADDR   "34:13:E8:5F:51:4E" // Bluetooth MAC address of server (run hcitool dev on server)
#define BT_TEAM_ID     1                   // team identifier placeholder

// Messages from SERVER TO CLIENT
#define BT_MSG_ACK		0
#define BT_MSG_START	1
#define BT_MSG_STOP		2
#define BT_MSG_KICK		3

// Messages from CLIENT TO SERVER
#define BT_MSG_POSITION	4
#define BT_MSG_MAPDATA	5
#define BT_MSG_MAPDONE 	6

/* Initializes bluetooth connection to server with address as defined in BT_SERV_ADDR.
 * 
 * Returns the socket through which communication will flow. */
int bt_connect(void);

/* Blocks until a message is recived from the server. If the message was of type START,
 * return 0. If the message is of any other type, return -1. */
int bt_wait_for_start(int sock);

/* Sends dummy x and y position every 2 seconds. Must be called from the same 
 * context as bt_connect! */
void bt_send_position(int sock);

/* 
Deprecated. Not using message queues.

Main bluetooth worker. Polls the provided bluetooth message queue (bt_mq) and parses
 * messages to the bluetooth server. 
 * 
 * Assumes bt_init() has been called and that START is received.
 * 
 * TODO: returns when server sends STOP or KICK.
 * 
 * 
 */
// void bt_client(int sock, mqd_t bt_mq);

#endif // !BT_CLIENT_H_