#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <mqueue.h>
#include <pthread.h>
#include <sys/types.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
#include <math.h>
#include "ev3_sensor.h"
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
// WIN32 /////////////////////////////////////////
#ifdef __WIN32__

#include <windows.h>

// UNIX //////////////////////////////////////////
#else
/*
#include <unistd.h>
#define Sleep( msec ) usleep(( msec ) * 1000 )

//////////////////////////////////////////////////
#endif
const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

#define MIN_STEP_VER 525 //minimum step covered going ahead in cm (25cm)
pthread_mutex_t mutex,mutex_pos;
*/

//////////////////////////////////
//         FOR BT               //
//////////////////////////////////
#define SERV_ADDR   "00:1E:10:00:06:2B"     /* address of the server is */
#define TEAM_ID     10                       /* team ID */

#define MSG_ACK     0
#define MSG_NEXT    1
#define MSG_START   2
#define MSG_STOP    3
#define MSG_CUSTOM  4
#define MSG_KICK    5
#define MSG_POSITION 6
#define MSG_BALL 	7
unsigned char rank = 0;
unsigned char length = 0;
unsigned char previous = 0xFF;
unsigned char next = 0xFF;
unsigned char side=0;
int s;
uint16_t msgId=0;

/*
typedef struct motandsens test;
struct motandsens {
	uint8_t sn;
	uint8_t dx;
	uint8_t med;
	uint8_t sn_touch;
	uint8_t sn_sonar;
	uint8_t sn_color;
	uint8_t sn_compass;
	uint8_t sn_mag;
        int max_speed;
	float x,y;
        int role;/*0 beg 1 fin*/
     //   int arena;/*0 small1 big*/
      //  int side;/*0 right 1 left*/
//	int number;	   
/*

};*/

//function to read from server
int read_from_server (int sock, char *buffer, size_t maxSize);
//function to send position to server
void send_pos_to_bt (void* args);

