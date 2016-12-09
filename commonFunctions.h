/* functions that are used in both small and big arena*/

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
// WIN32 /////////////////////////////////////////
#ifdef __WIN32__

#include <windows.h>

// UNIX //////////////////////////////////////////
#else

#include <unistd.h>
#define Sleep( msec ) usleep(( msec ) * 1000 )

//////////////////////////////////////////////////
#endif
const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

#define MIN_STEP_VER 525 //minimum step covered going ahead in cm (25cm)
pthread_mutex_t mutex;

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
        int arena;/*0 small1 big*/
        int side;/*0 right 1 left*/
    


};

struct pos {
	float x,y
	};
  
  //compass
  float get_compass_values(uint8_t sn_compass);
  void* position(void *args);
  // TURN RIGHT
  void rotatedx(uint8_t sn, uint8_t dx, uint8_t sn_compass, int max_speed, int rotation, uint8_t sn_mag);
  //TURN LEFT
  void rotatesx(uint8_t sn, uint8_t dx, uint8_t sn_compass, int max_speed, int rotation, uint8_t sn_mag);
  // keep a straight line
  void control_direction(uint8_t sn,uint8_t dx,uint8_t sn_compass,int max_speed, float initial_angle,uint8_t sn_mag);
  //grab the ball
  void grab_ball(uint8_t sn,uint8_t dx,uint8_t med,int max_speed);
  //color sensor
  int color_aq(uint8_t sn_color);
  //Leave the ball and stay up
  void leave_ball(uint8_t sn,uint8_t dx,uint8_t med,int max_speed);
  // put down, should be used after living the ball
  void put_down(uint8_t sn,uint8_t dx,uint8_t med,int max_speed);
  // go backward
  void go_backward(uint8_t sn,uint8_t dx,uint8_t med,int max_speed);
  // go aheah with different speeds depending on if an obstacle is detected near by or not
  float go_ahead_till_obstacle(uint8_t sn,uint8_t dx,int max_speed,uint8_t sn_sonar,int distance,uint8_t sn_compass, uint8_t sn_mag);
// color sensor
void* colorsense(void * args) ;
// initialize motors and sensors
   struct motandsens* inizialization (struct motandsens *donald);
   // search for the ball
   void research(uint8_t sn,uint8_t dx,int max_speed, uint8_t sn_compass, int max_turn_degree, uint8_t med, uint8_t sn_mag, uint8_t sn_sonar);
   
  
