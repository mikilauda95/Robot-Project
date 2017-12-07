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

static int bt_send_to_server(int type, char* data, int sock);
static int bt_read_from_server (int sock, char *buffer, size_t maxSize);

static int bt_send_to_server(int type, char* data, int sock) {
	static uint16_t msg_ID = 0;
	int msg_len = 0;
	char msg[BT_MSG_LEN_MAX];
	int status;

	if (type == BT_MSG_POSITION){
		msg[msg_len++] = (msg_ID >> 8) & 0xFF;
		msg[msg_len++] = msg_ID & 0xFF;
		msg[msg_len++] = BT_TEAM_ID;
		msg[msg_len++] = 0xFF;
		msg[msg_len++] = BT_MSG_POSITION;
		msg[msg_len++] = data[0];
		msg[msg_len++] = data[1];
		msg[msg_len++] = data[2];
		msg[msg_len++] = data[3];
		++msg_ID;
	} else {
		fprintf(stderr, "bt_send_message: unkown message type: %d\n", type);
		return -1;
	}

	status = write(sock, msg, msg_len);
	if (status != 0) {
		fprintf(stderr, "bt_send_message: failed to write to socket\n");
		return -1;
	}

	return 0;
}

int bt_wait_for_start(int sock) {
	char string[BT_MSG_LEN_MAX];
	bt_read_from_server(sock, string, 9);

	if (string[4] == BT_MSG_START) {
		return 0;
	} else {
		return -1;
	}

}

static int bt_read_from_server(int sock, char *buffer, size_t maxSize) {
	int bytes_read = read(sock, buffer, maxSize);

	if (bytes_read <= 0) {
		fprintf(stderr, "Server unexpectedly closed connection...\n");
		close(sock);
		exit(EXIT_FAILURE);
	}

	printf ("[DEBUG] received %d bytes\n", bytes_read);

	return bytes_read;
}

int bt_connect(void){
	struct sockaddr_rc addr = { 0 };
	int status, sock;

	// allocate a socket
	sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

	// set the connection parameters (who to connect to)
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = (uint8_t) 1;
	str2ba(BT_SERV_ADDR, &addr.rc_bdaddr);

	// connect to server
	status = connect(sock, (struct sockaddr *)&addr, sizeof(addr));

	// if connection failed
	if (status != 0) {
		fprintf(stderr, "bt_client: Failed to connect to server...\n");
	}
	printf("bt_client: conected to server %s\r\n", BT_SERV_ADDR);

	return sock;
}

void bt_client(int sock, mqd_t bt_mq) {

	/*
	 * This is where we would read from the message queue and call 
	 * bt_send_to_server. Due to the current limitations in the message
	 * queue, I will wait until we have implemented a more dynamic message
	 * queue that supports multi-chararcter messages. The below loop is 
	 * just there to keep the compiler happy.
	 * 														-Fredrk
	*/

	for(;;){
		uint16_t tmp[4];
		char tmp2[4];
		get_message(bt_mq, tmp, tmp);
		bt_send_to_server(BT_MSG_POSITION, tmp2, sock);
		Sleep(1);
	}

	printf("bt_client: worker exiting\n");

}