#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ev3.h"
#include "ev3_tacho.h"
#include "ev3_sensor.h"
#include "messages.h"
#include <pthread.h>
#include <math.h>
#include <signal.h>

#define LEFT_MOTOR_PORT 66
#define RIGHT_MOTOR_PORT 67
#define MAX_SPEED 1050
#define TIME_STEP 10//milliseconds
//old value
/*#define degree_to_lin 260/90*/
/*#define degree_to_lin 2.08*/
/*#define degree_to_lin 1.98*/
#define degree_to_lin 2.10
/*#define ANGULAR_SPEED 50*/
/*#define ANGULAR_SPEED 100*/
#define ANGULAR_SPEED 150


#define Sleep( msec ) usleep(( msec ) * 1000 )

enum name {L, R};
uint8_t motor[2];
uint8_t gyro_sn;
float gyro_value, ang_off;

/*uint8_t motor[0];*/
/*uint8_t motor[1];*/
pthread_t movid;
volatile int pos_flag;
pthread_mutex_t stopMutex;

double llin_speed, rlin_speed;
double rpos, lpos, distance=0;
double angle;
/*double coord[0], coord[1];*/
/*double coord[2];*/
struct coord {
	float x;
	float y;
} coord;

int movement_init(){
	ev3_tacho_init();
	ev3_sensor_init();

	ev3_search_tacho_plugged_in(LEFT_MOTOR_PORT, 0, &motor[L], 0 );
	ev3_search_tacho_plugged_in(RIGHT_MOTOR_PORT, 0, &motor[R], 0 );

	/* Decide how the motors should behave when stopping.
	   We have the alternatives COAST, BRAKE, and HOLD. They result in harder/softer breaking */
	//speed in cm per second
	set_tacho_stop_action_inx( motor[L], TACHO_BRAKE );
	set_tacho_stop_action_inx( motor[R], TACHO_BRAKE );

	set_tacho_speed_sp(motor[L], MAX_SPEED  / 2 );
	set_tacho_speed_sp(motor[R], MAX_SPEED / 2 );

	int rets=ev3_search_sensor( LEGO_EV3_GYRO	, &gyro_sn, 0 );
	if (rets) {
		printf("FOUND IT\n");
	}
	else {
		printf("NOT FOUND\n");
	}
	Sleep(1000);
	float a=0;
	for (int i = 0; i < 10; ++i) {
		get_sensor_value0(gyro_sn, &ang_off);
		a+=ang_off;
		Sleep(100);
	}
	ang_off=a/10;

	printf("OFFSET VALUE IS %f\n",ang_off);
	Sleep(1000);
	return 0;
}

static void _run_to_rel_pos( int l_speed, int l_pos, int r_speed, int r_pos )
{
	set_tacho_speed_sp( motor[L], l_speed );
	set_tacho_speed_sp( motor[R], r_speed );
	set_tacho_position_sp( motor[L], l_pos );
	set_tacho_position_sp( motor[R], r_pos );
	multi_set_tacho_command_inx( motor, TACHO_RUN_TO_REL_POS );
}



void TURN_DEGREES( int ang_speed, double angle ) {
	/*_run_to_rel_pos( ang_speed, -angle*degree_to_lin, ang_speed, angle*degree_to_lin);*/
	_run_to_rel_pos( ang_speed, angle*degree_to_lin, ang_speed, -angle*degree_to_lin);
	Sleep(1000);
	/*_wait_stopped = 1;*/ //dunno what this refers to so I keep it
}

void TURN_DEGREES_GYRO(int ang_speed, int angle)
{
	int i=0;
	float dest;
	/*float buffer[4];*/
	/*int j=0;*/
	float avg=0;
	/*for (i = 0; i < 4; ++i) {*/
	get_sensor_value0(gyro_sn,&gyro_value);
	printf("gyro VALUE before TURN : %f\n", gyro_value);
	/*}*/
	/*for (i = 0; i < 4; ++i) {*/
	/*avg+=buffer[i];*/
	/*}*/
	/*avg=avg/4;*/
	dest=(gyro_value+angle);
	/*if (dest>360) {*/
	/*dest-=360;*/
	/*}*/
	/*printf("Destination angle is %f and starting is %f\n",dest, gyro_value);*/
	if (angle>0) {
		set_tacho_speed_sp( motor[L], ang_speed );
		set_tacho_speed_sp( motor[R], -ang_speed );
		multi_set_tacho_command_inx(motor, TACHO_RUN_FOREVER);
	}
	else {
		set_tacho_speed_sp( motor[L], -ang_speed );
		set_tacho_speed_sp( motor[R], ang_speed );
		multi_set_tacho_command_inx(motor, TACHO_RUN_FOREVER);
	}
	/*j=0;*/
	while (1) {
		get_sensor_value0(gyro_sn,&gyro_value);
		/*avg=0;*/
		/*for (i = 0; i < 4; ++i) {*/
		/*avg+=buffer[i];*/
		/*}*/
		/*avg=avg/4;*/
		/*j++;*/
		printf("gyro VALUE is %f\n", gyro_value);
		if (angle<=0) {
			if (gyro_value<dest) {
				printf("FINAL gyro VALUE is %f\n", gyro_value);
				set_tacho_command_inx(motor[L], TACHO_STOP);
				set_tacho_command_inx(motor[R], TACHO_STOP);
				break;
			}
			else {
				printf("We were so close %f\n",fabs(gyro_value-dest));

			}
		}
		else {
			if (gyro_value>=dest) {
				printf("FINAL gyro VALUE is %f\n", gyro_value);
				set_tacho_command_inx(motor[L], TACHO_STOP);
				set_tacho_command_inx(motor[R], TACHO_STOP);
				break;
			}
			else {
				printf("We were so close %f\n",fabs(gyro_value-dest));

			}

		}
	}
}


void* tracepos()
{
	int rdistance, ldistance, rspeed,lspeed;
	double R;
	R=2.7;//5,4 of diameter//centimeter
	while (1) {
		/*pthread_mutex_lock(&stopMutex);*/
		if (pos_flag) {
			/*get_tacho_position_sp(motor[R], &ldistance);*/
			/*printf("The left distance is %d\n",ldistance);*/
			/*get_tacho_position_sp(motor[L], &rdistance);*/
			/*printf("The right distance is %d\n",rdistance);*/
			get_tacho_speed(motor[L], &lspeed);//tacho speed gives the degree per second
			/*printf("The left speed is %d\n",lspeed);*/
			get_tacho_speed(motor[L], &rspeed);
			/*printf("The right speed is %d\n",rspeed);*/
			/*get_tacho_count_per_rot(motor[L], &lspeed);*/
			/*printf("The left deg/s is %d\n",lspeed);*/
			/*get_tacho_count_per_rot(motor[L], &rspeed);*/
			/*printf("The right deg/s is %d\n",rspeed);*/
			llin_speed=lspeed*R*M_PI/180;//speed in cm per second
			/*printf("The left linear speed is %f\n",llin_speed);*/

			rlin_speed=rspeed*R*M_PI/180;//speed in cm per second
			/*printf("The right linear speed is %f\n",rlin_speed);*/
			get_sensor_value0(gyro_sn, &gyro_value);
			/*printf("gyro read is %f\n",gyro_value);*/
			lpos=llin_speed*TIME_STEP/1000;
			/*printf("lpos is %f\n",lpos);*/
			rpos=rlin_speed*TIME_STEP/1000;
			/*printf("rpos is %f\n",rpos);*/
			distance=(lpos+rpos)/2;
			/*distance=lpos;*/
			/*distance=rpos;*/
			angle=gyro_value-ang_off;
			/*printf("ANGLE IS %f\n",angle);*/
			/*printf("%f %f\n",angle, distance);*/
			coord.x+=distance*sin(angle*M_PI/180);
			coord.y+=distance*cos(angle*M_PI/180);

			/*printf("X= %f Y= %f\n",coord[0], coord[1]);*/
			/*printf("%f %f\n",coord.x, coord.y);*/
			Sleep(TIME_STEP);
		}
		else {
			distance=0;
			lpos=0;
			rpos=0;
			pthread_exit(NULL);
		}
	}
}

void stop(){
	set_tacho_command_inx(motor[L], TACHO_STOP);
	set_tacho_command_inx(motor[R], TACHO_STOP);
}
void forward(){
	set_tacho_speed_sp(motor[L], MAX_SPEED  / 2 );
	set_tacho_speed_sp(motor[R], MAX_SPEED / 2 );
	set_tacho_command_inx(motor[L], TACHO_RUN_FOREVER);
	set_tacho_command_inx(motor[R], TACHO_RUN_FOREVER);
}

void movement_start() {
	movement_init();

	// TESTING THE ANGLE PARAMETER= around 2.08 calibrated on 90 degrees
	pos_flag=1;
	/*if(pthread_create(&movid,NULL, &tracepos, NULL)!=0){*/
	/*printf("Failed the creation of the thread\n");*/
	/*}*/
	/*Sleep(1000);*/
	/*double angparam=2.10;*/
	/*int i;*/
	/*for (i = 0; i < 10; i+=1) {*/
	/*printf("ANG PARAM = %f\n",angparam);*/
	/*TURN_DEGREES(ANGULAR_SPEED, 90*angparam/2.08);*/
	/*TURN_DEGREES_GYRO(ANGULAR_SPEED*1/4, 90);*/
	/*Sleep(10000);*/
	/*TURN_DEGREES(ANGULAR_SPEED, -90*angparam/2.08);*/
	/*TURN_DEGREES_GYRO(ANGULAR_SPEED, 90);*/
	/*Sleep(5000);*/
	/*angparam-=0.05;*/
	/*}*/
	/*stop();*/
	/*if(pthread_create(&movid,NULL, &tracepos, NULL)!=0){*/
	/*printf("Failed the creation of the thread\n");*/
	/*}*/
	/*Sleep(1000);*/


	mqd_t movement_queue = init_queue("/movement", O_CREAT | O_RDONLY);
	for (int i = 0; i < 2; ++i) {

		forward();
		if(pthread_create(&movid,NULL, &tracepos, NULL)!=0){
			printf("Failed the creation of the thread\n");
		}
		Sleep(2500);
		stop();
		Sleep(2000);
		pos_flag=0;
		pthread_join(movid, NULL);
		/*printf("THESE ARE THE COORDINATES BEFORE TURNING %f %f\n",coord.x,coord.y);*/
		/*Sleep(5000);*/
		TURN_DEGREES_GYRO(ANGULAR_SPEED/2, 90);
		/*Sleep(5000);*/
		/*printf("THESE ARE THE COORDINATES AFTER TURNING %f %f\n",coord.x,coord.y);*/
		pos_flag=1;
		Sleep(2000);
		forward();
		if(pthread_create(&movid,NULL, &tracepos, NULL)!=0){
			printf("Failed the creation of the thread\n");
		}
		Sleep(1500);
		stop();
		Sleep(2000);
		pos_flag=0;
		pthread_join(movid, NULL);
		/*printf("THESE ARE THE COORDINATES BEFORE TURNING %f %f\n",coord.x,coord.y);*/
		/*Sleep(5000);*/
		TURN_DEGREES_GYRO(ANGULAR_SPEED/2, 90);
		/*Sleep(5000);*/
		/*printf("THESE ARE THE COORDINATES AFTER TURNING %f %f\n",coord.x,coord.y);*/
		pos_flag=1;
		Sleep(2000);
	}
}
