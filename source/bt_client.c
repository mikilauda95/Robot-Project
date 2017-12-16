#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <math.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "bt_client.h"

#define Sleep( msec ) usleep(( msec ) * 1000 )
#define BT_MSG_LEN_MAX 64

static int sock = -1;

static int _read_from_server(char *buffer, size_t maxSize) {
	int bytes_read = read(sock, buffer, maxSize);

	if (bytes_read <= 0) {
		fprintf(stderr, "Server unexpectedly closed connection...\n");
		close(sock);
		exit(EXIT_FAILURE);
	}

	return bytes_read;
}

static void _send_position(uint16_t x, uint16_t y) {
	
	static uint16_t msgId = 0;
	char string[58];

	*((uint16_t *) string) = msgId++;
	string[2] = BT_TEAM_ID;
	string[3] = 0xFF;
	string[4] = BT_MSG_POSITION;
	string[5] = x;
	string[6] = 0x00;
	string[7] = y;
	string[8] = 0x00;
	int ret = write(sock, string, 9);
	if (ret < 0) {
		fprintf(stderr, "BT: Failed to write to server!\r\n");
		return;
	}
	printf("BT: sending position (%d, %d)\r\n", x, y);

}


int bt_wait_for_start() {
	char string[BT_MSG_LEN_MAX];
	_read_from_server(string, 9);

	if (string[4] == BT_MSG_START) {
		return 0;
	} else {
		return -1;
	}

}

bool bt_connect(void) {
	struct sockaddr_rc addr = { 0 };
	int status;

	// allocate a socket
	sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// set the connection parameters (who to connect to)
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba(BT_SERV_ADDR, &addr.rc_bdaddr);

	printf("bt_connect: Attempting to connect...\r\n");
	status = connect(sock, (struct sockaddr *)&addr, sizeof(addr));

	if (status == 0) {
		printf("bt_client: conected to server %s with sock id %d \r\n", BT_SERV_ADDR, sock);
		return true;
	}else{
		fprintf(stderr, "bt_client: Failed to connect to server...\n");
		return false;
	}

}

void* bt_client(void *queues){

	mqd_t* tmp = (mqd_t*)queues;

	mqd_t bt_from_main_queue = tmp[0];

	uint16_t command;
	int value;
	uint16_t pos_x, pos_y;
	bool should_send = false;

	for(;;){
		
		get_message(bt_from_main_queue, &command, &value);

		switch(command) {
			case MESSAGE_POS_X:
				pos_x = value;
			break;
			case MESSAGE_POS_Y:
				pos_y = value;
				should_send = true;
			break;
		}

		if(should_send) {
			_send_position(pos_x, pos_y);
			should_send = false;
		}

	}

	return NULL;
}






















// void bt_client(int sock, mqd_t bt_mq) {

// 	/*
// 	 * This is where we would read from the message queue and call 
// 	 * bt_send_to_server. Due to the current limitations in the message
// 	 * queue, I will wait until we have implemented a more dynamic message
// 	 * queue that supports multi-chararcter messages. The below loop is 
// 	 * just there to keep the compiler happy.
// 	 * 														-Fredrk
// 	*/

// 	for(;;){
// 		uint16_t x = 100;
// 		uint16_t y = 120;
// 		bt_send_position(sock, x, y);
// 		Sleep(2000);
// 	}

// }













// uint16_t msgId = 0;
// static int bt_send_to_server(int type, char* data, int sock) {
// 	char string[58];
// 	printf ("I'm navigating...\n");

// 	if(type == BT_MSG_POSITION){
// 		*((uint16_t *) string) = msgId++;
// 		string[2] = BT_TEAM_ID;
// 		string[3] = 0xFF;
// 		string[4] = BT_MSG_POSITION;
// 		string[5] = 20;          /* x */
// 		string[6] = 0x00;
// 		string[7] = 50;              /* y */
// 		string[8]= 0x00;
// 		write(sock, string, 9);
// 	}
// }
// static int bt_send_to_server(int type, char* data, int sock) {
// 	static uint16_t msg_ID = 0;
// 	int msg_len = 0;
// 	char msg[BT_MSG_LEN_MAX];
// 	int status;

// 	if (type == BT_MSG_POSITION){
//     	*((uint16_t *) msg) = msg_ID++;
// 		msg[msg_len++] = BT_TEAM_ID;
// 		msg[msg_len++] = 0xFF;
// 		msg[msg_len++] = BT_MSG_POSITION;
// 		msg[msg_len++] = 0;
// 		msg[msg_len++] = 10;
// 		msg[msg_len++] = 0;
// 		msg[msg_len++] = 20;
// 		++msg_ID;
// 	} else {
// 		fprintf(stderr, "bt_send_message: unkown message type: %d\n", type);
// 		return -1;
// 	}

// 	printf("Sending data, %d bytes long\r\n", msg_len);
// 	status = write(sock, msg, 9);
// 	if (status != 0) {
// 		printf("write with status %d\r\n", status);
// 		fprintf(stderr, "bt_send_message: failed to write to socket: %d\n", sock);
// 		return -1;
// 	}

// 	return 0;
// }