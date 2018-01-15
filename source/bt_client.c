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

static void _send_mapdata(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
	static uint16_t msgId = 0;
	char string[58];

	*((uint16_t *) string) = msgId++;
	string[2] = BT_TEAM_ID;
	string[3] = 0xFF;
	string[4] = BT_MSG_MAPDATA;
	string[5] = x;
	string[6] = 0x00;
	string[7] = y;
	string[8] = 0x00;
	string[9] = r;
	string[10] = g;
	string[11] = b;
	int ret = write(sock, string, 12);
	if (ret < 0) {
		fprintf(stderr, "BT: Failed to write to server!\r\n");
		return;
	}
	printf("BT: sending MAPDATA (%d, %d) = (%d, %d, %d)\r\n", x, y, r, g, b);
}

static void _send_mapdone(void) {
	static uint16_t msgId = 0;
	char string[58];

	*((uint16_t *) string) = msgId++;
	string[2] = BT_TEAM_ID;
	string[3] = 0xFF;
	string[4] = BT_MSG_MAPDONE;
	int ret = write(sock, string, 5);
	if (ret < 0) {
		fprintf(stderr, "BT: Failed to write to server!\r\n");
		return;
	}
	printf("BT: send map done\n");
}

static void _send_obstacle(uint16_t x, uint16_t y) {
	static uint16_t msgId = 0;
	char string[58];

	*((uint16_t *) string) = msgId++;
	string[2] = BT_TEAM_ID;
	string[3] = 0xFF;
	string[4] = BT_MSG_OBSTACLE;
	string[5] = 0;
	string[6] = x;
	string[7] = 0x00;
	string[8] = y;
	string[9] = 0x00;
	int ret = write(sock, string, 10);
	if (ret < 0) {
		fprintf(stderr, "BT: Failed to write to server!\r\n");
		return;
	}
	printf("BT: send obstacle done\n");
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
	int16_t value;
	uint16_t pos_x, pos_y;
	uint16_t map_x_dim;
	uint16_t map_y_dim;
	uint16_t map_current_x, map_current_y;
	uint16_t drop_pair[2] = {0, 0};
	bool should_send_position = false;

	for(;;){
		
		get_message(bt_from_main_queue, &command, &value);

		switch(command) {
			case MESSAGE_POS_X:
				pos_x = value;
			break;
			case MESSAGE_POS_Y:
				pos_y = value;
				should_send_position = true;
			break;
			case MESSAGE_MAP_X_DIM:
				map_x_dim = value;
			break;
			case MESSAGE_MAP_Y_DIM:
				map_y_dim = value;
			break;
			case MESSAGE_MAP_POINT: {
				_send_mapdata(map_current_x, map_current_y, 255 * value, 255 * value, 255 * value);
				map_current_x++;
				if (map_current_x > map_x_dim - 1) {
					map_current_x = 0;
					map_current_y++;
				}
				if (map_current_y == map_y_dim - 1) {
					_send_mapdone();
					map_current_x = 0;
					map_current_y = 0;
				}
			}
			break;
			case MESSAGE_DROP_X:
			case MESSAGE_DROP_Y:
				drop_pair[command==MESSAGE_DROP_X?0:1]=(uint16_t)value;
				if (drop_pair[0] != 0 && drop_pair[1] != 0) {
					_send_obstacle(drop_pair[0], drop_pair[1]);
					drop_pair[0] = 0;
					drop_pair[1] = 0;
				}
			break;
		}

		if(should_send_position) {
			_send_position(pos_x, pos_y);
			should_send_position = false;
		}

	}

	return NULL;
}
