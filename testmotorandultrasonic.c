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

#include <unistd.h>
#define Sleep( msec ) usleep(( msec ) * 1000 )

//////////////////////////////////////////////////
#endif
const char const *color[] = { "?", "BLACK", "BLUE", "GREEN", "YELLOW", "RED", "WHITE", "BROWN" };
#define COLOR_COUNT  (( int )( sizeof( color ) / sizeof( color[ 0 ])))

#define MIN_STEP_VER 525 //minimum step covered going ahead in cm (25cm)
pthread_mutex_t mutex,mutex_pos;

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
        int arena;/*0 small1 big*/
        int side;/*0 right 1 left*/
	int number;	   


};

//calculate the the condition for the loop in the next function


////////////////////////////////////////////////////////////
/////			BT FUNCTIONS			////
/////			   START			////
////////////////////////////////////////////////////////////
int read_from_server (int sock, char *buffer, size_t maxSize) {
    int bytes_read = read (sock, buffer, maxSize);

    if (bytes_read <= 0) {
        fprintf (stderr, "Server unexpectedly closed connection...\n");
        close (s);
        exit (EXIT_FAILURE);
    }

    printf ("[DEBUG] received %d bytes\n", bytes_read);

    return bytes_read;
}
////////////////////////////////////////////////////////////
/////			BT FUNCTIONS			////
/////			   FINISH			////
////////////////////////////////////////////////////////////

//position
float get_compass_values(uint8_t sn_compass)
{
	int i;
	float sum;
	float degree;
	sum=0;
	for(i=0;i<50;i++)
	{
		
		get_sensor_value0(sn_compass, &degree);
		if ( !get_sensor_value0(sn_compass, &degree )) 
			{
			   degree = 0;
			} 
		
		
		Sleep(100);
		sum+=degree;
	}
	return sum/50;
	
}



float deg2rad(float m_rot)
{
	return m_rot * M_PI / 180.0;
}

float rad2deg(float m_rot)
{
	return m_rot * 180 / M_PI;
}

void positioning(void * args)
{	struct motandsens *donald = (struct motandsens *) args;	
	float encod_scale = M_PI * 5.5 / 360;
	float new_angs;
 	int retour;
	float m_rot,disp_diff;
 	static short flag = 0;
	static float last_angle  = 0;
	static float teta = 0;
	static float old_sx = 0;
	static float old_dx = 0;
	static float old_x = 0;
	static float old_y = 0;
 	float sign;
	int new_sx,new_dx;
	float disp_sx,disp_dx;
 	sign = 1;
 	retour = pthread_mutex_lock(&mutex_pos);
    			if (retour != 0)
    			 {
    			   perror("erreur mutex lock");
     			  exit(EXIT_FAILURE);
    			 }
	if ( !get_sensor_value0(donald->sn_mag, &new_angs )) 
	   {
	   new_angs = 0;
	   }
	retour = pthread_mutex_unlock(&mutex_pos);
    			if (retour != 0)
    			 {
    			   perror("erreur mutex unlock");
     			  exit(EXIT_FAILURE);
    			 }
	m_rot =  -(new_angs - last_angle);		//rotation
	m_rot = deg2rad(m_rot);				//rotation to rad
	last_angle = new_angs;				//refresh last angle
	get_tacho_position(donald->sn,&new_sx);			
	get_tacho_position(donald->dx,&new_dx);
	new_angs = deg2rad(new_angs);
	if(flag==1)
		teta = teta + m_rot;
 	else
	{
		teta = 0;
		flag = 1;
	}	
	disp_sx = new_sx - old_sx; 
	disp_dx = new_dx - old_dx;
	disp_diff = (disp_sx + disp_dx)*encod_scale/2;		//displacement
	old_sx = new_sx;
	old_dx = new_dx;
 	old_x = old_x + disp_diff * sign * sin( teta );
	old_y = old_y + disp_diff * sign * cos( teta ); 
 	donald->x = old_x;
 	donald->y = old_y;
	printf("y=%f and x=%f\n",old_y,old_x);
	
}

	void rotatedx(uint8_t sn, uint8_t dx, uint8_t sn_compass, int max_speed, int rotation, uint8_t sn_mag)
{	float actual_angle;
	float wanted_c;
	//set_tacho_position( sn,0);
	//set_tacho_position( dx,0);
	set_tacho_speed_sp( sn, max_speed/5);
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_speed_sp( dx, max_speed/5);
	set_tacho_ramp_up_sp( dx, 0 );
	set_tacho_ramp_down_sp( dx, 0 );
	set_tacho_position_sp( sn, 2 );
	set_tacho_position_sp( dx, -2);
	if ( !get_sensor_value0(sn_mag, &actual_angle )) {
                        actual_angle = 0;
		}
	wanted_c= actual_angle + rotation;

	while((actual_angle-wanted_c)<=0)
			{
			set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
			set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
			Sleep(50);
			if ( !get_sensor_value0(sn_mag, &actual_angle )) 
				{
                actual_angle = 0;
				}

			}
	

}

void rotatesx(uint8_t sn, uint8_t dx, uint8_t sn_compass, int max_speed, int rotation, uint8_t sn_mag)
{	float actual_angle;
	float wanted_c;
	//set_tacho_position( sn,0);
	//set_tacho_position( dx,0);
	set_tacho_speed_sp( sn, max_speed/5);
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_speed_sp( dx, max_speed/5);
	set_tacho_ramp_up_sp( dx, 0 );
	set_tacho_ramp_down_sp( dx, 0 );
	set_tacho_position_sp( sn,  -2);
	set_tacho_position_sp( dx, 2);
	if ( !get_sensor_value0(sn_mag, &actual_angle )) {
                        actual_angle = 0;
		}
	wanted_c= actual_angle - rotation;

	while((actual_angle-wanted_c)>=0)
			{
			set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
			set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
			Sleep(50);
			if ( !get_sensor_value0(sn_mag, &actual_angle )) 
				{
                actual_angle = 0;
				}

			}
	

}

void rotateforscan(uint8_t sn, uint8_t dx, int max_speed)
{
	set_tacho_speed_sp( sn, max_speed/2);
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_speed_sp( dx, max_speed/2);
	set_tacho_ramp_up_sp( dx, 0 );
	set_tacho_ramp_down_sp( dx, 0 );
	set_tacho_position_sp( sn,  -2);
	set_tacho_position_sp( dx, 2);
	set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
	set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
	Sleep(50);
}

//function that hold the direction
void control_direction(uint8_t sn,uint8_t dx,uint8_t sn_compass,int max_speed, float initial_angle,uint8_t sn_mag){
	float actual_angle;
	if ( !get_sensor_value0(sn_mag, &actual_angle)) {
		actual_angle = 0;
	}
	//printf("initial  %f\n",initial_angle);
	//printf("final %f\n", actual_angle);
	if(actual_angle!=initial_angle)
	{	
		if(actual_angle<(initial_angle - 2))	//too to the left turn right!!!
		{
			set_tacho_position_sp( sn,  1 );
			set_tacho_position_sp( dx, -1 );
			set_tacho_speed_sp( sn, max_speed/6 );
			set_tacho_speed_sp( dx, max_speed/6 );
			set_tacho_time_sp( sn, 100 );
			set_tacho_ramp_up_sp( sn,   0 );
			set_tacho_ramp_down_sp( sn, 0 );
			set_tacho_ramp_up_sp( dx,   0 );
			set_tacho_ramp_down_sp( dx, 0 );
			Sleep(100);
			while(actual_angle<initial_angle)
			{
				set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
				set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
				Sleep(100);
				if ( !get_sensor_value0(sn_mag, &actual_angle)){
				actual_angle=0;
				}

			}
			
		}
		if(actual_angle> (initial_angle + 2))	//too to the right turn left!!!
		{
			set_tacho_position_sp( sn, -1 );
			set_tacho_position_sp( dx,  1 );
			set_tacho_speed_sp( sn, max_speed/6 );
			set_tacho_speed_sp( dx, max_speed/6 );
			set_tacho_time_sp( sn, 100 );
			set_tacho_ramp_up_sp( sn,   0 );
			set_tacho_ramp_down_sp( sn, 0 );
			set_tacho_ramp_up_sp( dx,   0 );
			set_tacho_ramp_down_sp( dx, 0 );
			Sleep(100);
			while(actual_angle>initial_angle)
			{	
				set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
				set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
				Sleep(100);
				 if ( !get_sensor_value0(sn_mag, &actual_angle)){
				actual_angle=0;
				}
			}

		}

	}
}
//function that allow to grab the ball. Raise the grabber, go ahead till 5*22 mm. and than release the grabber and wait 
//few time till start moving
void grab_ball(uint8_t sn,uint8_t dx,uint8_t med,int max_speed)
{	
	int i;
	int retour;
	int act_pos,distance_el;
	set_tacho_time_sp( sn, 100 );
	set_tacho_ramp_up_sp( sn, 2000 );
	set_tacho_ramp_down_sp( sn, 2000 );
	set_tacho_time_sp( dx, 100 );
	set_tacho_ramp_up_sp( dx, 2000 );
	set_tacho_ramp_down_sp( dx, 2000 );
	set_tacho_speed_sp( sn, max_speed * 1 / 6 );
	set_tacho_speed_sp( dx, max_speed * 1 / 6 );
	//raise the grabber
	 retour = pthread_mutex_lock(&mutex);
	if (retour != 0)
	{
		perror("erreur mutex lock");
	        exit(EXIT_FAILURE);
	}
	set_tacho_position_sp( med, 90 );
	Sleep(200);
	for ( i = 0; i < 5; i++ ) 
	{
		set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
		Sleep( 200 );
	}
	get_tacho_position( dx, &act_pos);
	distance_el=act_pos;
	set_tacho_position_sp(sn,90);  //prova
	set_tacho_position_sp(dx,90);	//prova
	while((act_pos-(8*26)-distance_el)<=0)
	{	
		set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
		set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
		//set_tacho_command_inx( sn, TACHO_RUN_TIMED );
		//set_tacho_command_inx( dx, TACHO_RUN_TIMED );
		get_tacho_position( dx, &act_pos);
	}
	Sleep(200);
	//release the grabber
	set_tacho_position_sp( med, -50 );
	Sleep(200);
	for ( i = 0; i < 1; i++ ) 
	{
		set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
		Sleep( 200 );
	} 
	Sleep(1000);
	retour = pthread_mutex_unlock(&mutex);
	if (retour != 0)
	{
		perror("erreur mutex unlock");
		exit(EXIT_FAILURE);
	}
}   
int color_aq(uint8_t sn_color)
{	
	int i;
	int val,max,maxval;
	int mod[10];
	int count[8]={0};
	for(i=0;i<10;i++)
	{
		if ( !get_sensor_value( 0, sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) 
		{
			val = 0;
		}
		mod[i]=val;	
	}
	for(i=0;i<10;i++)
	{
		count[mod[i]]++;
	}
	max=0;
	maxval=0;
	for(i=0;i<8;i++)
	{
		if(count[i] > max)
		{
			max=count[i];
			maxval=i;
		}
	}
	return maxval;
}


void leave_ball(uint8_t sn,uint8_t dx,uint8_t med,int max_speed)
{			
	int i;
	set_tacho_time_sp( sn, 200 );
	set_tacho_ramp_up_sp( sn, 1500 );
	set_tacho_ramp_down_sp( sn, 1500 );
	set_tacho_time_sp( dx, 200 );
	set_tacho_ramp_up_sp( dx, 1500 );
	set_tacho_ramp_down_sp( dx, 1500 );
	set_tacho_speed_sp( sn, max_speed * 1 / 4 );
	set_tacho_speed_sp( dx, max_speed * 1 / 4 );
	//stabilize the ball
	Sleep(2000);
	//raise the grabber
	set_tacho_position_sp( med, 90 );
	Sleep(200);
	for ( i = 0; i < 7; i++ ) {
	set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
	Sleep( 200 );
	}
	//get_tacho_position( dx, &act_pos);
	//distance_el=act_pos;
	/*
	while((act_pos-(24*21)-distance_el)<=0)
	{
		set_tacho_command_inx( sn, TACHO_RUN_TIMED );
		set_tacho_command_inx( dx, TACHO_RUN_TIMED );
		get_tacho_position( dx, &act_pos);
	}
	//release the grabber
	set_tacho_position_sp( med, -90 );
	Sleep(200);
	for ( i = 0; i < 4; i++ ) {
	set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
	Sleep( 200 );
	} */
	Sleep(500);
 			
}  
void put_down(uint8_t sn,uint8_t dx,uint8_t med,int max_speed)
{			
	int i;
	//stabilize the ball
	set_tacho_position_sp( med, -90 );
	Sleep(200);
	for ( i = 0; i < 4; i++ ) {
	set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
	Sleep( 200 );
	} 
	Sleep(500);
 			
} 

void go_backward(uint8_t sn,uint8_t dx,uint8_t med,int max_speed)
{			
	int i;
	int distance_el;
	float act_pos;
	set_tacho_time_sp( sn, 800 );
	set_tacho_ramp_up_sp( sn, 2000 );
	set_tacho_ramp_down_sp( sn, 2000 );
	set_tacho_time_sp( dx, 800);
	set_tacho_ramp_up_sp( dx, 2000 );
	set_tacho_ramp_down_sp( dx, 2000 );
	set_tacho_speed_sp( sn, -max_speed * 1 / 6 );
	set_tacho_speed_sp( dx, -max_speed * 1 / 6 );
	//stabilize the ball
	Sleep(2000);
	get_tacho_position( dx, &act_pos);
	distance_el=act_pos;
	for(i=0;i<6;i++)
	{
		set_tacho_command_inx( sn, TACHO_RUN_TIMED );
		set_tacho_command_inx( dx, TACHO_RUN_TIMED );
		Sleep(500);
	}
	get_tacho_position( dx, &act_pos);
	Sleep(500);
 			
}  
void go_back(uint8_t sn,uint8_t dx,float distanceback,int max_speed,uint8_t sn_compass,uint8_t sn_mag)
{
float beginning, finish,partial;
float value,initial_angle;
set_tacho_time_sp( sn, 200 );
set_tacho_ramp_up_sp( sn, 1500 );
set_tacho_ramp_down_sp( sn, 1500);
set_tacho_time_sp( dx, 200 );
set_tacho_ramp_up_sp( dx, 1000 );
set_tacho_ramp_down_sp( dx, 1000 );
get_tacho_position( dx, &beginning);
if ( !get_sensor_value0(sn_mag, &initial_angle)){
                        initial_angle=0;
                        }
	
finish = beginning;
	
while(finish - beginning <= -distanceback)
{
	set_tacho_time_sp( sn, 200 );
	set_tacho_ramp_up_sp( sn, 1500 );
	set_tacho_ramp_down_sp( sn, 1500 );
	set_tacho_time_sp( dx, 200);
	set_tacho_ramp_up_sp( dx, 1500 );
	set_tacho_ramp_down_sp( dx, 1500 );
        set_tacho_speed_sp( sn, -max_speed * 1 / 6 );
	set_tacho_speed_sp( dx, -max_speed * 1 / 6 );
       	set_tacho_command_inx( sn, TACHO_RUN_TIMED );
	set_tacho_command_inx( dx, TACHO_RUN_TIMED );
        get_tacho_position( dx, &partial);
	control_direction(sn,dx,sn_compass,max_speed,initial_angle, sn_mag);
	get_tacho_position( dx, &finish);
	beginning+=(finish-partial);
}
}

float go_ahead_till_obstacle(uint8_t sn,uint8_t dx,int max_speed,uint8_t sn_sonar,int distance,uint8_t sn_compass, uint8_t sn_mag)
{	//aggiungere funzione che controlla anche il motore 
	//sinistro e vede se sono andati dritti tutti e due 
	//altrimenti significa che hai girato e c'è un errore
	//can be used to take the ball once detected 
	//we have to add the angle for the ball a routine to turn till this angle
	//and than go and take te ball
int beginning,finish,partial;
int retour;
float value;
float initial_angle;
set_tacho_time_sp( sn, 200 );
set_tacho_ramp_up_sp( sn, 1500 );
set_tacho_ramp_down_sp( sn, 1500);
set_tacho_time_sp( dx, 200 );
set_tacho_ramp_up_sp( dx, 1000 );
set_tacho_ramp_down_sp( dx, 1000 );
get_tacho_position( dx, &beginning);
finish = beginning;
if ( !get_sensor_value0(sn_mag, &initial_angle)){
                        initial_angle=0;
                        }
	
while((finish - beginning - distance)<=0){
	
	/*
	multi_set_tacho_time_sp( both, 100);
	multi_set_tacho_ramp_up_sp( both, 2000 );
	multi_set_tacho_ramp_down_sp( both, 2000 );
	*/
	set_tacho_time_sp( sn, 200 );
	set_tacho_ramp_up_sp( sn, 1500 );
	set_tacho_ramp_down_sp( sn, 1500 );
	set_tacho_time_sp( dx, 200);
	set_tacho_ramp_up_sp( dx, 1500 );
	set_tacho_ramp_down_sp( dx, 1500 );
	/*retour = pthread_mutex_lock(&mutex);
    			if (retour != 0)
    			 {
    			   perror("erreur mutex lock");
     			  exit(EXIT_FAILURE);
    			 }
	retour = pthread_mutex_unlock(&mutex);
    			if (retour != 0)
    			 {
    			   perror("erreur mutex unlock");
     			  exit(EXIT_FAILURE);
    			 }*/
	retour = pthread_mutex_lock(&mutex_pos);
    			if (retour != 0)
    			 {
    			   perror("erreur mutex lock");
     			  exit(EXIT_FAILURE);
    			 }
	retour = pthread_mutex_unlock(&mutex_pos);
    			if (retour != 0)
    			 {
    			   perror("erreur mutex unlock");
     			  exit(EXIT_FAILURE);
    			 }
	if ( !get_sensor_value0(sn_sonar, &value )) {
                                value = 0;
                        }
                        //printf( "\r(%f) \n", value);
			fflush( stdout );
     /*  if(value<2500 && value>=1500)
		{
	set_tacho_speed_sp( sn, max_speed );
	set_tacho_speed_sp( dx, max_speed );
			}
	if(value<1500 && value >=500)
		{
	set_tacho_speed_sp( sn, max_speed * 2 / 3 );
	set_tacho_speed_sp( dx, max_speed * 2 / 3 );
			}
	if(value<500 && value >=350)
		{
	set_tacho_speed_sp( sn, max_speed * 1 / 3 );
	set_tacho_speed_sp( dx, max_speed * 1 / 3 );
			       } */   
	if(value<2500 && value >=250)
		{
	set_tacho_speed_sp( sn, max_speed * 1 / 4 );
	set_tacho_speed_sp( dx, max_speed * 1 / 4 );
			       }         		
	if(value<250 && value >=70)
		{
	//multi_set_tacho_speed_sp(both, max_speed * 1 / 6);	
	set_tacho_speed_sp( sn, max_speed * 1 / 4 );
	set_tacho_speed_sp( dx, max_speed * 1 / 4 );
			       }
	if(value<70 && value >=40)
		{
	//multi_set_tacho_speed_sp(both, max_speed * 1 / 24);	
	set_tacho_speed_sp( sn, max_speed * 1 / 24 );
	set_tacho_speed_sp( dx, max_speed * 1 / 24);
		 }
	if(value<40 && value >=0)
		 {
		printf("sono nello zero\n");
		fflush( stdout );
	 //multi_set_tacho_speed_sp(both, max_speed * 0);	
	 set_tacho_speed_sp( sn, max_speed * 0 );
	 set_tacho_speed_sp( dx, max_speed * 0 );
	 Sleep(100);
	 break;
		}
	//multi_set_tacho_command_inx(both , TACHO_RUN_TIMED );
	set_tacho_command_inx( sn, TACHO_RUN_TIMED );
	set_tacho_command_inx( dx, TACHO_RUN_TIMED );
	Sleep(100);
	get_tacho_position( dx, &partial);
	control_direction(sn,dx,sn_compass,max_speed,initial_angle, sn_mag);
	get_tacho_position( dx, &finish);
	beginning+=(finish-partial);
}
get_tacho_position( dx, &finish);		
control_direction(sn,dx,sn_compass,max_speed,initial_angle, sn_mag);	
return (finish-beginning)/21; //return the distance in cm
}
void* positioning_sys(void* args)
{	int seconds_bt=0;
 	char string[58];
 	int16_t x_conv,y_conv;
 	int8_t x_conv_MSB,x_conv_LSB,y_conv_MSB,y_conv_LSB;
	struct motandsens *donald = (struct motandsens *) args;	
	while(1)
	{
	positioning(donald);
	Sleep(100);
	seconds_bt=seconds_bt+1;
	if (seconds_bt == 20)
	{	//send position
	 	x_conv_MSB = (0xFF & ((int16_t)donald->x>>8));
	 	x_conv_LSB = (0xFF &  ((int16_t)donald->x));
		y_conv_MSB = (0xFF & ((int16_t)donald->y>>8));
		y_conv_LSB = (0xFF &  ((int16_t)donald->y));
		printf("x: %d x: %d\n",x_conv_LSB,x_conv_MSB);
		printf("y: %d y: %d\n",y_conv_LSB,y_conv_MSB);
		*((uint16_t *) string) = msgId++;
		string[2] = TEAM_ID;
		string[3] = 0xFF;
		string[4] = MSG_POSITION;
		string[5] = x_conv_LSB;          // x 
		string[6] = x_conv_MSB;
		string[7] = y_conv_LSB;	    // y 
		string[8] = y_conv_MSB;
		write(s, string, 9);
		seconds_bt = 0;
	}
	}
}
void gotoxyfinisher(float xoldf, float yoldf,float xnewf, float ynewf, uint8_t sn,uint8_t dx,int max_speed,uint8_t sn_sonar, uint8_t sn_compass, uint8_t sn_mag)
{
	float deltax, deltay ,distanceto, angleofrotation;
	deltax=(xnewf-xoldf);
	deltay=(ynewf-yoldf);
	distanceto=sqrt(pow(deltax,2)+pow(deltay,2))*19;
	printf("distance to do: %f",distanceto);
	//angleofrotationback=atan((double)(abs(deltax)/abs(deltay)))*180/M_PI;
	deltax=(double)abs(deltax);
	deltay=(double)abs(deltay);
	angleofrotation=atan2(deltax,deltay)*180/M_PI;
	printf("angle of turning : %f",angleofrotation);
	rotatedx(sn,dx,sn_compass,max_speed,angleofrotation,sn_mag);
	go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,distanceto,sn_compass,sn_mag);


}
int colorsense(uint8_t sn,uint8_t dx, uint8_t med, int max_speed, uint8_t sn_color)
{ 	int val;
 	int stricol[10];
 	//struct motandsens *donald = (struct motandsens *) args;
 	if ( !get_sensor_value( 0, sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
				val = 0;
			}
			strcpy(stricol,color[ color_aq(sn_color) ]);
			printf("stricolo: %s\n", stricol );
			if(( strcmp(stricol,"RED")==0) || ( strcmp(stricol,"GREEN")==0))
			{
				grab_ball(sn,dx,med,max_speed);
				Sleep(200);
				return 1;
				
			}
	return 0;
}
void* movements(void * args)
{
	/*	We decide a certain movements to do in order to go from the beginning to the destination and than
		at certain point we scan for the ball.
		Than after we found the ball we can go till the ball, than return back, and restart to follow fixed 
		trajectory, without searching the ball.	
		THE TRAJECTORY CHOSEN IS GO 1 TIME AHEAD FOR 10 CM + 2 TIMES AHEAD FOR 25 CM +
		TURN LEFT + 1 TIME AHEAD FOR 10 CM + 2 TIMES AHEAD FOR 25 CM + TURN RIGHT + 
		2 TIMES AHEAD FOR 25 C */
int i;
struct motandsens *donald = (struct motandsens *) args;
float degree;
//int arena;
int found=0; //this is a flag used to know if the ball has been detected 0=NO 1=YES
//arena  case 0 TEST #1 go straight ahead
//arena  case 1 TEST #2 leave the ball at the center
//arena = 6;
float heading;
float POS_Y=0;
float POS_X=0;
float angleofrotationback;
double deltax,deltay;
float xbefore,ybefore,distanceback;
if ( !get_sensor_value0(donald->sn_mag, &heading)){
					heading=0;
					}
switch(donald->number)
{
	case 0 :
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,2745,donald->sn_compass, donald->sn_mag);
		break;
	case 1 :
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1432,donald->sn_compass, donald->sn_mag);
		//TURN LEFT
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,89,donald->sn_mag);
		Sleep(1000);
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,600,donald->sn_compass,donald->sn_mag); 	
		printf("I'am in movements after turn\n");	
		Sleep(500);
		leave_ball(donald->sn,donald->dx,donald->med,donald->max_speed);
		Sleep(1000);
		go_backward(donald->sn,donald->dx,donald->med,donald->max_speed);
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		put_down(donald->sn,donald->dx,donald->med,donald->max_speed);		
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1240,donald->sn_compass, donald->sn_mag);
		//put_down(donald->sn,donald->dx,donald->med,donald->max_speed);
		Sleep(1000);
		break;
	case 2:

		Sleep(500); //time elapsed to scan

		// research(donald->sn, donald->dx,donald->max_speed, donald->sn_compass,0, donald->med, donald->sn_mag, donald->sn_sonar);
		/*if(research(sn, dx, max_speed, sn_compass, 45)==1)
			{	
				found=1;
					//funtion to take the ball and return back}
			else
				*/
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,MIN_STEP_VER,donald->sn_compass, donald->sn_mag);

		printf("I'am in movements\n");
		for(i=0;i<2;i++)
		{
		/*if(found != 1)
			if(research(sn, dx, max_speed, sn_compass, 90)==1)
			{	
				found=1;
					//funtion to take the ball and return back}
			else*/
				go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,MIN_STEP_VER,donald->sn_compass,donald->sn_mag);

				printf("I'am in movements' for1\n");
				Sleep(1000);
		}
		//TURN LEFT
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		Sleep(1000);
			if(found != 1)
			/*
			if(research(sn, dx, max_speed, sn_compass, 90)==1)
			{	
				found=1;
					//funtion to take the ball and return back}
			else*/	
				go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,MIN_STEP_VER-315,donald->sn_compass,donald->sn_mag); 	

				printf("I'am in movements after turn\n");	
				Sleep(1000);

		//	for(i=0;i<2;i++)
		//	{
		/*if(found != 1)
			if(research(sn, dx, max_speed, sn_compass, 90)==1)
			{	
				found=1;
					//funtion to take the ball and return back}
			else*/
		//			go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,MIN_STEP_VER,sn_compass);
		//			printf("I'am in movements turn left\n");
		//			Sleep(1000);
		//	}

		//TURN RIGHT
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed, 180, donald->sn_mag);
		Sleep(1000);
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,MIN_STEP_VER-315,donald->sn_compass,donald->sn_mag);
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		Sleep(1000);
		for(i=0;i<2;i++)
		{
		/*if(found != 1)
			if(research(sn, dx, max_speed, sn_compass, 90)==1)
			{	
				found=1;
					//funtion to take the ball and return back}
			else*/
				go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,MIN_STEP_VER,donald->sn_compass,donald->sn_mag);

				printf("I'am in movements turn right\n");
				Sleep(1000);
		}
		/*if(found != 1)
			if(research(sn, dx, max_speed, sn_compass, 90)==1)
			{	
				found=1;
					//funtion to take the ball and return back}
			else*/
				go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,MIN_STEP_VER-295,donald->sn_compass,donald->sn_mag);


				printf("I'am in movements finish\n");
				Sleep(1000);
		//WE HOPE ARRIVED HOME
		get_sensor_value0(donald->sn_compass, &degree);
		break;
	case 4 : 
		// go from beginning starting area to the beginner destination area without getting bump right side
		//move from 1m// 19*90 =1710
		//initial_pos=get_compass_values(donald->sn_compass);
		
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1710,donald->sn_compass, donald->sn_mag);
		//TURN RIGHT to avoid first obstacle
		
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,88,donald->sn_mag);
		
		//move from 1m to the right
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1368,donald->sn_compass,donald->sn_mag); 	
		//printf("I'am in movements after turn\n");	
	
		//TURN LEFT
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,86,donald->sn_mag);
		// go until obstacle around 1m (TO TEST !!! and mesure on the arena)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1745,donald->sn_compass,donald->sn_mag); 
		//TURN LEFT to avoid second obstacle
	
		rotatesx(donald->sn,donald->dx,donald->sn_compass,(donald->max_speed)/2,90,donald->sn_mag);
		// go around 1m (TO TEST !!! and mesure on the arena)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1180, donald->sn_compass,donald->sn_mag);
		//TURN RIGHT 
		//final_pos=get_compass_values(donald->sn_compass);
		/*if(final_pos>initial_pos)
			turn_pos=359-final_pos+initial_pos;	   		
		else	
			turn_pos=initial_pos-final_pos;*/
		
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,87,donald->sn_mag);
		// go until final base
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,3220,donald->sn_compass,donald->sn_mag);
		
		break;
		
	case 5 :
		// go from beginning starting area to the beginner destination area without getting bump left side
		
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1710,donald->sn_compass, donald->sn_mag);
		//TURN RIGHT to avoid first obstacle
		
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		
		//move from 1m to the right
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1368,donald->sn_compass,donald->sn_mag); 	
		//printf("I'am in movements after turn\n");	
	
		//TURN LEFT
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,89,donald->sn_mag);
		// go until obstacle around 1m (TO TEST !!! and mesure on the arena)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1615,donald->sn_compass,donald->sn_mag); 
		//TURN LEFT to avoid second obstacle
	
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,87,donald->sn_mag);
		// go around 1m (TO TEST !!! and mesure on the arena)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1140, donald->sn_compass,donald->sn_mag);
		//TURN RIGHT 
		//final_pos=get_compass_values(donald->sn_compass);
		/*if(final_pos>initial_pos)
			turn_pos=359-final_pos+initial_pos;	   		
		else	
			turn_pos=initial_pos-final_pos;*/
		
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		// go until final base
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,3250,donald->sn_compass,donald->sn_mag);
		
		break;
		
	case 6 :
	//big arena right, begin at down corner test 4 : place the ball 
		//move from 1m
	//initial_pos=get_compass_values(donald->sn_compass);
	
	go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1710,donald->sn_compass, donald->sn_mag);
		//TURN RIGHT to avoid first obstacle
		
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		
		//move from 1m to the right
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1368,donald->sn_compass,donald->sn_mag); 	
		
		//TURN LEFT
		
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		
		// go until obstacle around 1m (TO TEST !!! and mesure on the arena)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1765,donald->sn_compass,donald->sn_mag); 
		//TURN LEFT to avoid second obstacle
		
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,87,donald->sn_mag);
		
		// go after ball area around 60 cm (TO TEST !!! and mesure on the arena)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1159,donald->sn_compass,donald->sn_mag);
		//turn around 
		
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,180,donald->sn_mag);
		
		//move to ball area (maybe won't be needed, depend on how much it goes backward)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,370,donald->sn_compass,donald->sn_mag);
		// drop the ball in the ball area
		leave_ball(donald->sn,donald->dx,donald->med,donald->max_speed);
		
		go_backward(donald->sn,donald->dx,donald->med,donald->max_speed);
		//TURN AROUND
		/*final_pos=get_compass_values(donald->sn_compass);
		if(final_pos<initial_pos)
	   		turn_pos=359-initial_pos+final_pos;
		else	
			turn_pos=final_pos-initial_pos;*/
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,84,donald->sn_mag);
		put_down(donald->sn,donald->dx,donald->med,donald->max_speed);	
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,3230,donald->sn_compass, donald->sn_mag);
		
		
		break;	
		
		case 7 :
	//big arena left, begin at down corner test 4 : place the ball 
		//move from 1m
	go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1710,donald->sn_compass, donald->sn_mag);
		//TURN RIGHT to avoid first obstacle
		
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		
		//move from 1m to the right
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1368,donald->sn_compass,donald->sn_mag); 	
		
		//TURN LEFT
		
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		
		// go until obstacle around 1m (TO TEST !!! and mesure on the arena)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1705,donald->sn_compass,donald->sn_mag); 
		//TURN LEFT to avoid second obstacle
		
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,90,donald->sn_mag);
		
		// go after ball area around 60 cm (TO TEST !!! and mesure on the arena)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,1159,donald->sn_compass,donald->sn_mag);
		//turn around 
		
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,180,donald->sn_mag);
		
		//move to ball area (maybe won't be needed, depend on how much it goes backward)
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,370,donald->sn_compass,donald->sn_mag);
		// drop the ball in the ball area
		leave_ball(donald->sn,donald->dx,donald->med,donald->max_speed);
		
		go_backward(donald->sn,donald->dx,donald->med,donald->max_speed);
		//TURN AROUND
		/*final_pos=get_compass_values(donald->sn_compass);
		if(final_pos<initial_pos)
	   		turn_pos=359-initial_pos+final_pos;
		else	
			turn_pos=final_pos-initial_pos;*/
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,84,donald->sn_mag);
		put_down(donald->sn,donald->dx,donald->med,donald->max_speed);	
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,3230,donald->sn_compass, donald->sn_mag);
		
		
		break;	
		
		case 8 :
	//big arena right, begin at up corner test : grab the ball 
	
		Sleep(1000);
		break;	
		
		
		case 9 :
	//big arena left, begin at up corner test : grab the ball 
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,722,donald->sn_compass, donald->sn_mag);
		//TURN LEFT
		Sleep(300);
		xbefore=donald->x;
		ybefore=donald->y;
		printf("---------------- x and y before turning : x=%f , y=%f\n", xbefore,ybefore);
		rotatesx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,45,donald->sn_mag);
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,240,donald->sn_compass, donald->sn_mag);
	        research(donald->sn,donald->dx, donald->max_speed, donald->sn_compass, 45 , donald->med, donald->sn_color, donald->sn_mag, donald->sn_sonar);
		////////////////////////////////////////////////////////
		Sleep(300);
                printf("--------- x and y after ball taken : x=%f , y=%f\n", donald->x,donald->y);
		deltax=(donald->x)-xbefore;
		deltay=(donald->y)-ybefore;
		distanceback=sqrt(pow(((donald->x)-xbefore),2)+pow(((donald->y)-ybefore),2))*19;
		printf("distance to come back: %f",distanceback);
		//angleofrotationback=atan((double)(abs(deltax)/abs(deltay)))*180/M_PI;
		deltax=(double)abs(deltax);
		deltay=(double)abs(deltay);
		angleofrotationback=atan2(deltax,deltay)*180/M_PI;
		printf("angle of turning back: %f",angleofrotationback);
		
		go_back(donald->sn,donald->dx,distanceback,donald->max_speed,donald->sn_compass,donald->sn_mag);
		rotatedx(donald->sn,donald->dx,donald->sn_compass,donald->max_speed,angleofrotationback,donald->sn_mag);
		gotoxyfinisher(donald->x, donald->y, 0.0, 200.0,donald->sn,donald->dx,donald->max_speed,donald->sn_sonar, donald->sn_compass, donald->sn_mag);
		Sleep(1000);
		break; 
	case 10 :
		while(1)
		{
		go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,190,donald->sn_compass, donald->sn_mag);
		POS_X = POS_X * cos(heading);
		POS_Y = POS_Y * sin(heading);
		if ( !get_sensor_value0(donald->sn_mag, &heading)){
					heading=0;
					}
		printf("POS_X=%f\n",POS_X);
		printf("POS_Y=%f\n",POS_Y);
		printf("heading=%f\n",heading);
		}
		break;
	case 11:
	gotoxyfinisher(donald->x, donald->y, 40.0, 80.0,donald->sn,donald->dx,donald->max_speed,donald->sn_sonar, donald->sn_compass, donald->sn_mag);	
	break;	
}
return EXIT_SUCCESS;
}

struct motandsens* inizialization (struct motandsens *donald)
{
int i;
char s[ 256 ];
int val;
float value;
uint32_t n, ii;
	int prova1, prova2, prova3;
	/*
 	printf( "Found tacho motors:\n" );
        for ( i = 0; i < DESC_LIMIT; i++ ) {
                if ( ev3_tacho[ i ].type_inx != TACHO_TYPE__NONE_ ) {
                        printf( "  type = %s\n", ev3_tacho_type( ev3_tacho[ i ].type_inx ));
                        printf( "  port = %s\n", ev3_tacho_port_name( i, s ));
                }
        }*/
	prova1=ev3_search_tacho_plugged_in(65,0, &donald->dx, 0 );
	Sleep(100);
	prova2=ev3_search_tacho_plugged_in(68,0, &donald->sn, 0 );
	Sleep(100);
	prova3=ev3_search_tacho_plugged_in(67,0, &donald->med, 0 );
	Sleep(100);
	printf("prova 1 = %d \n", prova1);
	Sleep(500);
	printf("prova 2 = %d \n", prova2);
	Sleep(500);

	printf("prova 3 = %d \n", prova3);
	Sleep(500);
      //if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &donald->sn, 0 )){
                get_tacho_max_speed( donald->sn, &donald->max_speed );
                printf("value of buffer :%d\n", donald->sn);
                printf("  max_speed = %d\n", donald->max_speed );
                set_tacho_stop_action_inx( donald->sn, TACHO_COAST );
                set_tacho_polarity( donald->sn, "normal" );
                set_tacho_speed_sp( donald->sn, donald->max_speed * 2 / 3 );
                set_tacho_time_sp( donald->sn, 100 );
                set_tacho_ramp_up_sp( donald->sn, 2000 );
                set_tacho_ramp_down_sp( donald->sn, 2000 );
		set_tacho_position( donald->sn,0);
               // set_tacho_command_inx( donald->sn, TACHO_RUN_TIMED );
                /* Wait tacho stop */
                Sleep( 100 );

    /*    } else {
                printf( "LEGO_EV3_L_MOTOR 1 is NOT found\n" );
                fflush( stdout );
        }*/
//Second motor
/*if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &donald->dx, 1 )) {
                printf( "LEGO_EV3_L_MOTOR 2 is found, run for 5 sec...\n" );
  */             
                set_tacho_stop_action_inx( donald->dx, TACHO_COAST );
                set_tacho_polarity( donald->dx, "normal" );
                set_tacho_speed_sp( donald->dx, donald->max_speed * 2 / 3 );
                set_tacho_time_sp( donald->dx, 100 );
                set_tacho_ramp_up_sp( donald->dx, 2000 );
	        set_tacho_ramp_down_sp( donald->dx, 2000 );
		set_tacho_position( donald->dx,0);
                //set_tacho_command_inx( donald->dx, TACHO_RUN_TIMED );
                /* Wait tacho stop */
                Sleep( 100 );
		fflush( stdout );

      /*  } else {
                printf( "LEGO_EV3_L_MOTOR 2 is NOT found\n" );
		fflush( stdout );
        }*/
//medium motor
	//if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &donald->med, 2 )) {
		

	//	printf( "LEGO_EV3_L_MOTOR 3 is found, \n" );
		
		set_tacho_stop_action_inx( donald->med, TACHO_COAST );
		set_tacho_polarity( donald->med, "normal" );
		set_tacho_speed_sp( donald->med, donald->max_speed/12);
		set_tacho_time_sp( donald->med, 6000 );
		set_tacho_ramp_up_sp( donald->med, 2000 );
		set_tacho_ramp_down_sp( donald->med, 2000 );
		//set_tacho_command_inx( donald->med, TACHO_RUN_TIMED );
		/* Wait tacho stop */
		Sleep( 100 );
	
	/*} else {
		printf( "LEGO_EV3_L_MOTOR 1 is NOT found\n" );
		fflush( stdout );
	} */
	
//Run all sensors
        ev3_sensor_init();

        printf( "Found sensors:\n" );
        for ( i = 0; i < DESC_LIMIT; i++ ) {
                if ( ev3_sensor[ i ].type_inx != SENSOR_TYPE__NONE_ ) {
                        printf( "  type = %s\n", ev3_sensor_type( ev3_sensor[ i ].type_inx ));
                        printf( "  port = %s\n", ev3_sensor_port_name( i, s ));
			fflush( stdout );
                        if ( get_sensor_mode( i, s, sizeof( s ))) {
                                printf( "  mode = %s\n", s );
                        }
                        if ( get_sensor_num_values( i, &n )) {
                                for ( ii = 0; ii < n; ii++ ) {
                                        if ( get_sensor_value( ii, i, &val )) {
                                                printf( "  value%d = %d\n", ii, val );
                                        }
                                }
                        }
                }
	}
        if ( ev3_search_sensor( LEGO_EV3_TOUCH, &donald->sn_touch, 0 )) {
                //printf( "TOUCH sensor is found, press BUTTON for EXIT...\n" );
        }

                if (ev3_search_sensor(HT_NXT_COMPASS, &donald->sn_compass,0)){
                        //printf("COMPASS found, reading compass...\n");
                        if ( !get_sensor_value0(donald->sn_compass, &value )) {
                        value = 0;
                        }
                        printf( "compass\r(%f) \n", value);
                        fflush( stdout );
                }
                if ( ev3_search_sensor( LEGO_EV3_COLOR, &donald->sn_color, 0 )) {
			printf( "COLOR sensor is found, setting...\n" );
			set_sensor_mode( donald->sn_color, "COL-COLOR" );
			if ( !get_sensor_value( 0, donald->sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
				val = 0;
			}
			printf( "\r(%s) \n", color[ val ]);
			printf( "valore del colore e': %d\n",val);
			fflush( stdout );
		}
                if (ev3_search_sensor(LEGO_EV3_US, &donald->sn_sonar,0)){
                        //printf("SONAR found, reading sonar...\n");
                        if ( !get_sensor_value0(donald->sn_sonar, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
			fflush( stdout );
                }
                if (ev3_search_sensor(LEGO_EV3_GYRO, &donald->sn_mag,0)){
                        //printf("GYRO found, reading magnet...\n");
			set_sensor_mode( donald->sn_mag, "GYRO-ANG" );
                        if ( !get_sensor_value0(donald->sn_mag, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
                        fflush( stdout );
		}
donald->x=0;
donald->y=0;
return donald;
}

void research(uint8_t sn,uint8_t dx,int max_speed, uint8_t sn_compass, int max_turn_degree, uint8_t med, uint8_t sn_color,uint8_t sn_mag, uint8_t sn_sonar)
{	//Take the initial position than move to 
float initial_angle;
float start_angle, final_angle, middle_angle,turn_angle;
int pos_in_sn, pos_in_dx, pos_in_ball_sn, pos_in_ball_dx; 
int pos_fin_ball_sn, pos_fin_ball_dx, found_sn, found_dx;
int i, flag_1,flag_2,grab, ball_dist, status_re;
float points[1000]={0};
if ( !get_sensor_value0(sn_mag, &initial_angle )) 
   {
   initial_angle = 0;
   }
get_tacho_position(sn, &pos_in_sn);
get_tacho_position(dx, &pos_in_dx);
status_re = 0;
while(status_re==0)
	{
		flag_1=0;
		flag_2=0;  // because of vibration the first value scanned after first angle must be cecked
		//turn right 45 ° and start moving 2° each step
		rotatedx(sn,dx,sn_compass,max_speed,35,sn_mag);	
		Sleep(200);
		for(i=0;i<90;i++)
		{
			//printf("I'M here\n");
			Sleep(500);
			get_sensor_value0(sn_sonar, &points[i]);
			if(flag_1==1 && flag_2==0) //if you have found the ball the first value after that can be wrong due to vibrations
			{
				if(points[i] > points[i-1] + 200)
					points[i]=points[i-1];
				flag_2=1;
			}
				
			printf("Il valore è %f:\n",points[i]);
			if(i!=0 && ((points[i-1]-points[i])>=300) && flag_1==0)
			{
			 //this is the first balls' extremity 
				if ( !get_sensor_value0(sn_mag, &start_angle )) 
				{
				start_angle = 0;
				}
				printf("first_angle%f\n",start_angle);
				get_tacho_position(sn,&pos_in_ball_sn);
				get_tacho_position(dx,&pos_in_ball_dx);
				flag_1=1;
			}
			if(i!=0 && ((points[i]-points[i-1])>=300) && flag_1==1)
			{
			  //this is the last point of the ball detected
				if ( !get_sensor_value0(sn_mag, &final_angle )) 
				{
				final_angle = 0;
				}
				printf("final_angle%f\n",final_angle);
				get_tacho_position(sn,&pos_in_ball_sn);
				get_tacho_position(dx,&pos_in_ball_dx);
				ball_dist = points[i-2];
				flag_1=2;

			}
			if(flag_1==2)
			{	
				middle_angle = (final_angle + start_angle) / 2;
				printf("middle_angle%f\n",middle_angle);
				found_sn=(pos_fin_ball_sn - pos_in_ball_sn) / 2;
				found_dx=(pos_fin_ball_dx - pos_in_ball_dx) / 2;
				break;
			}
			//rotatesx(sn,dx,sn_compass,max_speed,1,sn_mag);
			rotateforscan(sn,dx,max_speed);
			
		}

		

		//it has finished the search
		//restart from centre and go to the desired angle
		/*if(final_angle<0)
			turn_angle = middle_angle - final_angle;
		else
			turn_angle=final_angle-middle_angle;*/
		turn_angle = abs( middle_angle - final_angle );
		rotatedx(sn,dx,sn_compass,max_speed,turn_angle,sn_mag); 
		Sleep(200);
		
		if (ball_dist > 300)
		{	
			go_ahead_till_obstacle(sn,dx,max_speed/2,sn_sonar,ball_dist/2,sn_compass,sn_mag);
			for(i=0;i<1000;i++)
				points[i]=0; //TO BE CONTROLLED
		}
		else
		{
			status_re=1; //LEAVE THE RESEARCH TAKE THE BALL
		}

}
/*rotatesx(sn,dx,sn_compass,max_speed,middle_angle,sn_mag);
elapsed_dis=go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,4000,sn_compass,sn_mag);
rotatedx(sn,dx,sn_compass,max_speed,180,sn_mag);*/
grab=colorsense(sn,dx,med,max_speed,sn_color);
printf("grab=%d\n",grab);
while(grab==0)
{
	go_ahead_till_obstacle(sn,dx,max_speed/2,sn_sonar,95,sn_compass,sn_mag);
	grab=colorsense(sn,dx,med,max_speed,sn_color);
	printf("grab=%d\n",grab);
}
//colorsense(sn,dx,med,max_speed,sn_color);
/*rotatesx(sn,dx,sn_compass,max_speed,180,sn_mag);
rotatedx(sn,dx,sn_compass,max_speed,middle_angle,sn_mag);*/
//hope it will work=)
return;
}


	
int main( int argc, char **argv )
{	pid_t ret;
 	char *name;
        int i,d,n;
	struct motandsens *donald=malloc(sizeof(struct motandsens));
        FLAGS_T state;
        int val;
	int act_pos;
        float value;      
 	int retour;		
	pthread_t thread_movement, thread_position, thread_colorsense; 
        pthread_mutex_init(&mutex, NULL);
 	int caseNumber;
#ifndef __ARM_ARCH_4T__
        /* Disable auto-detection of the brick (you have to set the correct address below) */
        ev3_brick_addr = "192.168.0.204";

#endif
        if ( ev3_init() == -1 ) return ( 1 );
#ifndef __ARM_ARCH_4T__
        printf( "The EV3 brick auto-detection is DISABLED,\nwaiting %s online with plugged tacho...\n", ev3_brick_addr );

#else
        printf( "Waiting tacho is plugged...\n" );

#endif
        while ( ev3_tacho_init() < 1 ) Sleep( 1000 );

 

 
 	caseNumber = atoi(argv[1]);
 
        printf( "*** ( EV3 ) Hello! ***\n" );
	
	donald = inizialization(donald);
 	donald->number = caseNumber;
 	///////////////////////////////////////////////////////////////////////
	////								   ////
	////			CONNECTION TO SERVER			   ////
	////								   ////
	///////////////////////////////////////////////////////////////////////
    struct sockaddr_rc addr = { 0 };
    int status;
    
    /* allocate a socket */
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    /* set the connection parameters (who to connect to) */
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba (SERV_ADDR, &addr.rc_bdaddr);

    /* connect to server */
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    /* if connected */
    if( status == 0 ) {
        char string[58];

        /* Wait for START message */
        read_from_server (s, string, 9);
        if (string[4] == MSG_START) {
            printf ("Received start message!\n");
            rank = (unsigned char) string[5];
	    side = (unsigned char) string[6];
            next = (unsigned char) string[7];
			
        }
		if (side==0){
		    printf("I am on the right side\n");
		}
		else {
			printf("I am on the left side\n");
		}
		
        if (rank == 0){
            printf("beginner\n");
	    }
        else
            printf("beginner\n");



    } else {
        fprintf (stderr, "Failed to connect to server...\n");
        sleep (2);
	close(s);
        exit (EXIT_FAILURE);
    }

   
	///////////////////////////////////////////////////////////////////////
	////								   ////
	////			CONNECTION ESTABLISHED			   ////
	////								   ////
	///////////////////////////////////////////////////////////////////////

	retour = pthread_create(&thread_movement, NULL, movements, donald);
	if (retour != 0)
	{
	  perror("erreur thread movement");
	  exit(EXIT_FAILURE);
	}
 
 	retour = pthread_create(&thread_position, NULL, positioning_sys, donald);
	if (retour != 0)
	{
	  perror("erreur thread sensor");
	  exit(EXIT_FAILURE);
	}
 	/*retour = pthread_create(&thread_colorsense, NULL, colorsense, donald);
	if (retour != 0)
	{
	  perror("erreur thread sensor");
	  exit(EXIT_FAILURE);
	}*/
 	
	if (pthread_join(thread_position, NULL)) 
	{
	  perror("pthread_join position");
	  return EXIT_FAILURE;
	}
	if (pthread_join(thread_movement, NULL)) 
	{
	  perror("pthread_join movement");
	  return EXIT_FAILURE;
	} 
 	/*if (pthread_join(thread_colorsense, NULL)) 
	{
	  perror("pthread_join colorsens");
	  return EXIT_FAILURE;
	} 	*/
        ev3_uninit();
        printf( "*** ( EV3 ) Bye! ***\n" );

        return ( 0 );       

}
