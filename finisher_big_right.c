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
#define SERV_ADDR   "98:01:A7:9F:09:00"     /* address of the server is */
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
int vett[100];
float x_ball, y_ball;

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
	float teta;
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

float get_sonar_values(uint8_t sn_sonar)
{
	int i;
	float sum;
	float sonar_val;
	sum=0;
	for(i=0;i<5;i++)
	{
		

		if ( !get_sensor_value0(sn_sonar, &sonar_val)) 
			{
			   sonar_val = 0;
			} 
		
		Sleep(50);
		
		sum+=sonar_val;
	}
	return sum/5;
	
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
	static float last_angle  = 0;//prima era 180
	static float teta_calc = -M_PI/2;//prima era M_PI
	static float old_sx = 0;
	static float old_dx = 0;
	static float old_x = 84;
	static float old_y = 390;//finisher 190
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
 	//printf("new_angs: %f   last_angle: %f\n",new_angs, last_angle);
	m_rot = -( new_angs - last_angle);		//rotation
 	//printf("m_rot: %f\n",m_rot);
	m_rot = deg2rad(m_rot);				//rotation to rad
	last_angle = new_angs;				//refresh last angle
	get_tacho_position(donald->sn,&new_sx);			
	get_tacho_position(donald->dx,&new_dx);
	new_angs = deg2rad(new_angs);
	if(flag==1)
		teta_calc = teta_calc + m_rot;
 	else
	{
		teta_calc = -M_PI/2;  //-M_PI/2 per il finisher
		flag = 1;
	}	
	disp_sx = new_sx - old_sx; 
	disp_dx = new_dx - old_dx;
	disp_diff = (disp_sx + disp_dx)*encod_scale/2;		//displacement
	old_sx = new_sx;
	old_dx = new_dx;
 	old_x = old_x + disp_diff * sign * cos( teta_calc );
	old_y = old_y + disp_diff * sign * sin( teta_calc ); 
 	//printf("teta calc: %f:\n",teta_calc);
 	donald->teta=teta_calc;
 	donald->x = old_x;
 	donald->y = old_y;
	//printf("y=%f and x=%f\n",old_y,old_x);
	
}


void rotatedx(uint8_t sn, uint8_t dx, int max_speed, int rotation, uint8_t sn_mag)
{	float actual_angle;
	float wanted_c;
	//set_tacho_position( sn,0);
	//set_tacho_position( dx,0);
	set_tacho_speed_sp( sn, max_speed/2);
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_speed_sp( dx, max_speed/2);
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

void rotatesx(uint8_t sn, uint8_t dx, int max_speed, int rotation, uint8_t sn_mag)
{	float actual_angle;
	float wanted_c;
	//set_tacho_position( sn,0);
	//set_tacho_position( dx,0);
	set_tacho_speed_sp( sn, max_speed/2);
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_speed_sp( dx, max_speed/2);
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

void rotateforscan(uint8_t sn, uint8_t dx, int max_speed, int vers)
{
	set_tacho_speed_sp( sn, max_speed/2);  //vers +1 left -1 right
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_speed_sp( dx, max_speed/2);
	set_tacho_ramp_up_sp( dx, 0 );
	set_tacho_ramp_down_sp( dx, 0 );
	set_tacho_position_sp( sn,  -2*vers);
	set_tacho_position_sp( dx, 2*vers);
	set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
	set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
	Sleep(50);
}

//function that hold the direction
void control_direction(uint8_t sn,uint8_t dx,int max_speed, float initial_angle,uint8_t sn_mag){
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
void put_down(uint8_t med,int max_speed)
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
	control_direction(sn,dx,max_speed,initial_angle, sn_mag);
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
     	if(value<2500 && value >=250)
		{
	set_tacho_speed_sp( sn, max_speed * 1 / 3 );
	set_tacho_speed_sp( dx, max_speed * 1 / 3 );
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
	control_direction(sn,dx,max_speed,initial_angle, sn_mag);
	get_tacho_position( dx, &finish);
	beginning+=(finish-partial);
}
get_tacho_position( dx, &finish);		
control_direction(sn,dx,max_speed,initial_angle, sn_mag);	
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
	if(seconds_bt == 20 && donald->number==1)
		seconds_bt=0;
	if (seconds_bt == 20 && donald->number==0)
	{	//send position
	 	x_conv_MSB = (0xFF & ((int16_t)donald->x>>8));
	 	x_conv_LSB = (0xFF &  ((int16_t)donald->x));
		y_conv_MSB = (0xFF & ((int16_t)donald->y>>8));
		y_conv_LSB = (0xFF &  ((int16_t)donald->y));
		//printf("x: %d x: %d\n",x_conv_LSB,x_conv_MSB);
		//printf("y: %d y: %d\n",y_conv_LSB,y_conv_MSB);
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
	//printf("distance to do: %f",distanceto);
	//angleofrotationback=atan((double)(abs(deltax)/abs(deltay)))*180/M_PI;
	deltax=(double)abs(deltax);
	deltay=(double)abs(deltay);
	angleofrotation=atan2(deltax,deltay)*180/M_PI;
	//printf("angle of turning : %f",angleofrotation);
	rotatedx(sn,dx,max_speed,angleofrotation,sn_mag);
	go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,distanceto,sn_compass,sn_mag);


}
void gotoxybeg(float xoldf, float yoldf,float xnewf, float ynewf, uint8_t sn,uint8_t dx,int max_speed,uint8_t sn_sonar, uint8_t sn_compass, uint8_t sn_mag, float teta)
{
	
	float deltax, deltay ,distanceto, angleofrotation, rot;
	deltax=(xnewf-xoldf);
	deltay=(ynewf-yoldf);
	distanceto=sqrt(pow(deltax,2)+pow(deltay,2))*19;
	//printf("distance to do: %f",distanceto);
	//angleofrotationback=atan((double)(abs(deltax)/abs(deltay)))*180/M_PI;
	deltax=(double)deltax;
	deltay=(double)deltay;
	angleofrotation=atan2(deltax,deltay)*180/M_PI;  
	//printf("teta is :%f\n",teta);
	//printf("delta aangle is :%f\n",angleofrotation); 
	rot = -90+rad2deg(teta) + angleofrotation;		//solo per prova era -360+ teta+angle ofrotation
	//printf("angle of turning : %f",rot);
	if (rot<-180)
		rot=rot+360;
	if (rot>180)
		rot=rot-360;
	if(rot<0)
		rotatesx(sn,dx,max_speed,abs(rot),sn_mag);
	else
		rotatedx(sn,dx,max_speed,rot,sn_mag);
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
char string[58];
int16_t x_conv,y_conv;
int8_t x_conv_MSB,x_conv_LSB,y_conv_MSB,y_conv_LSB;
if ( !get_sensor_value0(donald->sn_mag, &heading)){
					heading=0;
					}
while(donald->number==1) //wait your turn
	{
	    Sleep(200);
	}
Sleep(1000);
gotoxybeg(donald->x, donald->y, vett[0], vett[1],donald->sn,donald->dx,donald->max_speed,donald->sn_sonar, donald->sn_compass, donald->sn_mag, donald->teta);	
rotatesx(donald->sn,donald->dx,donald->max_speed,vett[2],donald->sn_mag);
go_ahead_till_obstacle(donald->sn,donald->dx,donald->max_speed,donald->sn_sonar,vett[3],donald->sn_compass,donald->sn_mag);
research(donald->sn,donald->dx, donald->max_speed, donald->sn_compass, vett[4] , donald->med, donald->sn_color, donald->sn_mag, donald->sn_sonar);
x_ball = donald->x+5*cos(donald->teta);
y_ball = donald->y+5*sin(donald->teta);
x_conv_MSB = (0xFF & ((int16_t)x_ball>>8));
x_conv_LSB = (0xFF &  ((int16_t)x_ball));
y_conv_MSB = (0xFF & ((int16_t)y_ball>>8));
y_conv_LSB = (0xFF & ((int16_t)y_ball));
	*((uint16_t *) string) = msgId++;
			string[2] = TEAM_ID;
			string[3] = next;
			string[4] = MSG_BALL;
			string[5] = 1; // robot leaved the ball
			string[6] = x_conv_LSB;          // x 
			string[7] = x_conv_MSB;
			string[8] = y_conv_LSB;	    // y 
			string[9] = y_conv_MSB;
write(s, string, 10);
gotoxybeg(donald->x, donald->y, vett[5], vett[6],donald->sn,donald->dx,donald->max_speed,donald->sn_sonar, donald->sn_compass, donald->sn_mag, donald->teta);
gotoxybeg(donald->x, donald->y, vett[7], vett[8],donald->sn,donald->dx,donald->max_speed,donald->sn_sonar, donald->sn_compass, donald->sn_mag, donald->teta);

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
			set_sensor_mode( donald->sn_mag, "GYRO-CAL" );
			set_sensor_mode( donald->sn_mag, "GYRO-ANG" );
			
                        if ( !get_sensor_value0(donald->sn_mag, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
                        fflush( stdout );
		}
//FINISHER
donald->x=84; 
donald->y=390;//finisher=190
donald->teta=-90;//finisher=-90
donald->number=1;
return donald;
}

void research(uint8_t sn,uint8_t dx,int max_speed, uint8_t sn_compass, int max_turn_degree, uint8_t med, uint8_t sn_color,uint8_t sn_mag, uint8_t sn_sonar)
{	//Take the initial position than move to 
float initial_angle;
float start_angle, final_angle, middle_angle,turn_angle,end_angle;
int pos_in_sn, pos_in_dx, pos_in_ball_sn, pos_in_ball_dx,init_turn; 
int pos_fin_ball_sn, pos_fin_ball_dx, found_sn, found_dx;
int i, flag_1,flag_2,grab, ball_dist, status_re;
int angles_to_scan=30;
float points[1000]={8000000};
float angle[1000]={0};
init_turn = max_turn_degree;
//init_turn=35;   TO BE CHANGED WITH 
if ( !get_sensor_value0(sn_mag, &initial_angle )) 
   {
   initial_angle = 0;
   }
get_tacho_position(sn, &pos_in_sn);
get_tacho_position(dx, &pos_in_dx);
status_re = 0;
flag_1=0;
while(status_re==0)
	{
		
		//flag_2=0;  // because of vibration the first value scanned after first angle must be cecked
		//turn right 45 ° and start moving 2° each step
		rotatesx(sn,dx,max_speed,init_turn,sn_mag);	
		Sleep(200);
		for(i=0;i<angles_to_scan;i++)
		{	points[i]=get_sonar_values(sn_sonar);
			get_sensor_value0(sn_mag, &angle[i] );
			rotateforscan(sn,dx,max_speed,-1);
		}
		int max=8000000;
		int index=0;
	      	for(i=0;i<angles_to_scan;i++)
		{	
		  if(points[i]<max)
		  {
			  max=points[i];
			  index=i;
		  }
		}
		printf("point[index]:%f\n",points[index]);
		printf("angle[index]:%f\n",angle[index]);
	if ( !get_sensor_value0(sn_mag, &final_angle)) 
   {
   final_angle = 0;
   }
	middle_angle=abs(angle[index]-final_angle);
	printf("final angle:%f and middle angle:%f\n",final_angle,middle_angle);
	rotatesx(sn,dx,max_speed,middle_angle-4,sn_mag);
	if (flag_1==0)
	{
		go_ahead_till_obstacle(sn,dx,max_speed*2/3,sn_sonar,points[index]*3/4,sn_compass,sn_mag);
		flag_1=1;
		for(i=0;i<500;i++)
			{
				points[i]=800000;
			}
	}
	else
	{
		status_re=1;
		init_turn=25;
		angles_to_scan=20;
	}
}
grab=colorsense(sn,dx,med,max_speed,sn_color);
printf("grab=%d\n",grab);
while(grab==0)
{
	go_ahead_till_obstacle(sn,dx,max_speed*2/3,sn_sonar,40,sn_compass,sn_mag);
	grab=colorsense(sn,dx,med,max_speed,sn_color);
	printf("grab=%d\n",grab);
}
return;
}

void research2(uint8_t sn,uint8_t dx,int max_speed, uint8_t sn_compass, int max_turn_degree, uint8_t med, uint8_t sn_color,uint8_t sn_mag, uint8_t sn_sonar)
{	//Take the initial position than move to 
float initial_angle;
float start_angle, final_angle, middle_angle,turn_angle,end_angle;
int pos_in_sn, pos_in_dx, pos_in_ball_sn, pos_in_ball_dx,init_turn; 
int pos_fin_ball_sn, pos_fin_ball_dx, found_sn, found_dx;
int i, flag_1,flag_2,grab, ball_dist, status_re;
int angles_to_scan=20;
float points[1000]={8000000};
float angle[1000]={0};
init_turn = max_turn_degree;
//init_turn=35;   TO BE CHANGED WITH 
if ( !get_sensor_value0(sn_mag, &initial_angle )) 
   {
   initial_angle = 0;
   }
get_tacho_position(sn, &pos_in_sn);
get_tacho_position(dx, &pos_in_dx);
status_re = 0;
flag_1=0;
while(status_re==0)
	{
		
		//flag_2=0;  // because of vibration the first value scanned after first angle must be cecked
		//turn right 45 ° and start moving 2° each step
		rotatedx(sn,dx,max_speed,init_turn,sn_mag);	
		Sleep(200);
		for(i=0;i<angles_to_scan;i++)
		{	points[i]=get_sonar_values(sn_sonar);
			get_sensor_value0(sn_mag, &angle[i] );
			rotateforscan(sn,dx,max_speed,1);
		}
		int max=8000000;
		int index=0;
	      	for(i=0;i<angles_to_scan;i++)
		{	
		  if(points[i]<max)
		  {
			  max=points[i];
			  index=i;
		  }
		}
		printf("point[index]:%f\n",points[index]);
		printf("angle[index]:%f\n",angle[index]);
	if ( !get_sensor_value0(sn_mag, &final_angle)) 
   {
   final_angle = 0;
   }
	middle_angle=abs(angle[index]-final_angle);
	printf("final angle:%f and middle angle:%f\n",final_angle,middle_angle);
	rotatedx(sn,dx,max_speed,middle_angle-4,sn_mag);
	if (flag_1==0)
	{
		go_ahead_till_obstacle(sn,dx,max_speed*2/3,sn_sonar,points[index]*3/4,sn_compass,sn_mag);
		flag_1=1;
		for(i=0;i<500;i++)
			{
				points[i]=800000;
			}
	}
	else
	{
		status_re=1;
	}
}
grab=colorsense(sn,dx,med,max_speed,sn_color);
printf("grab=%d\n",grab);
while(grab==0)
{
	go_ahead_till_obstacle(sn,dx,max_speed*2/3,sn_sonar,40,sn_compass,sn_mag);
	grab=colorsense(sn,dx,med,max_speed,sn_color);
	printf("grab=%d\n",grab);
}
return;
}

	
int main()
{	//ONLY FOR MAPPING ARENA
	
	FILE *fd;	
 	// FINISH MAPPING	
	pid_t ret;
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
	 int game_status_flag=0;	// is set to one if a kick message or a stop message is received
	char string[58];
	char ack[10];
int8_t x_LSB,x_MSB,y_MSB,y_LSB;
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

	//ONLY FOR MAPPING
	/* apre il file */
	  fd=fopen("maps.txt", "r"); 
			/* verifica errori in apertura */
	  if( fd==NULL ) {
	    perror("Errore in apertura del file");
	    exit(1);
	  }
		
	  fscanf(fd, "%d %d %d %d %d %d %d %d %d %d %d %d",&vett[0],&vett[1],&vett[2],&vett[3],&vett[4],&vett[5],&vett[6],&vett[7],&vett[8],&vett[9],&vett[10],&vett[11]);
	  fclose(fd);
	
	
	//FINISH MAPPING
 
        printf( "*** ( EV3 ) Hello! ***\n" );
	
	donald = inizialization(donald);
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

    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    /* if connected */
    while( status != 0 )
    {
	printf("I'm connecting....\n\n\n\n");    
    }
			
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

 	
        
 while(game_status_flag==0)
 {
 read_from_server (s, string, 9);
 printf("message type %d\n",string[4]);
  switch (string[4])	
 { 	
	case MSG_START:
            printf ("Received start message!\n");
            rank = (unsigned char) string[5];
	    side = (unsigned char) string[6];
            next = (unsigned char) string[7];
	    if (rank== 0)
	    {
		    printf("I am beginner\n");
		    donald->role = 0;	//only at the init to know how to start
		    donald->number = 0 ;
		    //THE BEGINNER CAN START
			
	    }
	    else 
	    {
		    printf("I am finisher\n");
		    donald->role = 1;
		    donald->number =  1;
		    //WAIT NEXT MESSAGE
	    }
 	    printf("##########################################\n##########    Next is %d    ###########\n#########################################\n\n",next);
	    break;
	case MSG_BALL:
    
            		printf ("Received ball message! Ball has been left \n");
           		string[2] = TEAM_ID;
			x_LSB = string[6];          // get x of the ball
			x_MSB = string[7];
			y_LSB = string[8];	    //get y of the ball
			y_MSB = string[9];
		        *((uint16_t *) ack) = msgId++;
			ack[2] = TEAM_ID;
			ack[3] = next;
			ack[4] = MSG_ACK;
			ack[5] = string[0]; // Id ack 
			ack[6] = string[1]; // Id ack
			ack[7] = 0; // 0 if it OK, 1 if it failed 
			write(s, ack, 8);
	      break;
	 case MSG_KICK :
	     game_status_flag =1;
	     if (string[5] == 10)
             return (1);
	     break;
	 case MSG_STOP:
	     game_status_flag =1;
	     return (1);
	     break;
	 case MSG_NEXT:
	     printf("NEXT RECEIVED!!!\n");
	     donald->number =  0;
	     *((uint16_t *) ack) = msgId++;
	     ack[2] = TEAM_ID;
	     ack[3] = next;
	     ack[4] = MSG_ACK;
	     ack[5] = string[0]; // Id ack 
	     ack[6] = string[1]; // Id ack
	     ack[7] = 0; // 0 if it OK, 1 if it failed 
	     write(s, ack, 8);
	     //OK let start!!
	     break;
             
	     
        }
				
}
   
	///////////////////////////////////////////////////////////////////////
	////								   ////
	////			CONNECTION ESTABLISHED			   ////
	////								   ////
	///////////////////////////////////////////////////////////////////////


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
        ev3_uninit();
        printf( "*** ( EV3 ) Bye! ***\n" );

        return ( 0 );       

}
