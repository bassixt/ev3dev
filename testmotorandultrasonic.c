#include <stdio.h>
#include <string.h>
#include "ev3.h"
#include "ev3_port.h"
#include "ev3_tacho.h"
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

#define MIN_STEP_VER 20 //minimum step covered going ahead in cm
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

};

struct pos {
	float x,y
	};
//function that hold the direction
void control_direction(uint8_t sn,uint8_t dx,uint8_t sn_compass,int max_speed, float initial_angle){
		float actual_angle;
		if ( !get_sensor_value0(sn_compass, &actual_angle )) {
                        actual_angle = 0;
		}
		if(actual_angle!=initial_angle)
		{	
			if(actual_angle<(initial_angle - 2))	//too to the left turn right!!!
			{
				set_tacho_position_sp( sn,  2 );
				set_tacho_position_sp( dx, -2 );
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
					if ( !get_sensor_value0(sn_compass, &actual_angle )) {
					actual_angle = 0;
					}
				}
			
			}
			if(actual_angle> (initial_angle + 2))	//too to the right turn left!!!
			{
				set_tacho_position_sp( sn, -2 );
				set_tacho_position_sp( dx,  2 );
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
					if ( !get_sensor_value0(sn_compass, &actual_angle )) {
					actual_angle = 0;
					}
				}
			
			}
			
		}
}

//function that allows to rotate on the right side
void rotatedx(uint8_t sn,uint8_t dx,uint8_t sn_compass,int max_speed, int rotation){
		//int i;
		float actval;
		float degree;
		float ins,ind;
		int destro, sinistro;
		set_tacho_position( sn,0);
		set_tacho_position( dx,0);
		set_tacho_speed_sp( sn, max_speed/2);
		set_tacho_ramp_up_sp( sn, 0 );
		set_tacho_ramp_down_sp( sn, 0 );
		set_tacho_speed_sp( dx, max_speed/2);
		set_tacho_ramp_up_sp( dx, 0 );
		set_tacho_ramp_down_sp( dx, 0 );
		set_tacho_position_sp( sn, -5 );
		set_tacho_position_sp( dx, 5);
		Sleep(100);
		get_sensor_value0(sn_compass, &degree);
		//printf("Initial position %s\n", degree );
		//initial=degree;
		//while((degree-initial-rotation)<=0)
		//{
		get_tacho_position(sn, &sinistro);
		get_tacho_position(dx, &destro);
		ins=sinistro;
		ind=destro;
		actval=rotation*3;
		printf("sinistro %d\n",sinistro);
		printf("destro %d\n",destro);
			//for(i=0;i<410;i++)
			 	while((destro-ind)<=actval||(sinistro-ins)>=-actval)
				{	
				set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
				set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
				Sleep( 50 );
				get_tacho_position(sn, &sinistro);
				get_tacho_position(dx, &destro);
				}
									      
			Sleep(500);
			get_sensor_value0(sn_compass, &degree);
			//printf("Final position %s\n", degree );
			
			get_tacho_position(sn, &sinistro);
			get_tacho_position(dx, &destro);
			printf("sinistro %d\n",sinistro);
			printf("destro %d\n",destro);
			fflush( stdout );
	
}
//function that allows to rotate on the left side
void rotatesx(uint8_t sn,uint8_t dx,uint8_t sn_compass,int max_speed, int rotation){
		//int i;
		float degree;
		float initial;		
		set_tacho_speed_sp( sn, max_speed/12);
		set_tacho_ramp_up_sp( sn, 0 );
		set_tacho_ramp_down_sp( sn, 0 );
		set_tacho_position_sp( sn, 1);
		set_tacho_speed_sp( dx, max_speed/12);
		set_tacho_ramp_up_sp( dx, 0 );
		set_tacho_ramp_down_sp( dx, 0 );
		set_tacho_position_sp( dx, 5 );
			set_tacho_position_sp( sn, 90);
			set_tacho_position_sp( dx, -90 );
			Sleep(200);
			get_sensor_value0(sn_compass, &degree);
			//this function avoid the problem due to the initial position e.g. initial = 40
			//expected rotation 90 degree final destination 310...
			initial=degree-rotation;
			if (initial<0)
			{	
				initial=359+initial;// If the number is negative trasnlate it in a positive one
				while(degree<359)
				{	
					if(degree<=359&&degree>355)	
						{break;}
					else
						{	
						set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
						set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
						get_sensor_value0(sn_compass, &degree);
						}Sleep( 200 );
				}
				while(degree>=initial)
				{			
				set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
				set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
				get_sensor_value0(sn_compass, &degree);
				}Sleep( 200 );
			}
			else{
			while((degree-initial)>=0)//it can rotate freely becouse the final pos is positive
				{			
				set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
				set_tacho_command_inx( dx, TACHO_RUN_TO_REL_POS );
				get_sensor_value0(sn_compass, &degree);
				}
			    Sleep( 200 );
			}



}
//function that allow to grab the ball. Raise the grabber, go ahead till 5*22 mm. and than release the grabber and wait 
//few time till start moving
void grab_ball(uint8_t sn,uint8_t dx,uint8_t med,int max_speed)
{			int i;
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
			set_tacho_position_sp( med, 90 );
			Sleep(200);
			for ( i = 0; i < 7; i++ ) {
			set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
			Sleep( 200 );
			}
 			get_tacho_position( dx, &act_pos);
 			distance_el=act_pos;
 			while((act_pos-(5*26)-distance_el)<=0)
			{
				set_tacho_command_inx( sn, TACHO_RUN_TIMED );
				set_tacho_command_inx( dx, TACHO_RUN_TIMED );
				get_tacho_position( dx, &act_pos);
			}
			//release the grabber
			set_tacho_position_sp( med, -90 );
			Sleep(200);
			for ( i = 0; i < 5; i++ ) {
			set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
			Sleep( 200 );
			} 
 			Sleep(1000);
}   
int color_aq(uint8_t sn_color)
{	
	int i;
	int val,max,maxval;
	int mod[10];
	int count[8]={0};
	for(i=0;i<10;i++)
	{
	if ( !get_sensor_value( 0, sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
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
{			int i;
			int act_pos,distance_el;
			set_tacho_time_sp( sn, 100 );
			set_tacho_ramp_up_sp( sn, 2000 );
			set_tacho_ramp_down_sp( sn, 2000 );
			set_tacho_time_sp( dx, 100 );
			set_tacho_ramp_up_sp( dx, 2000 );
			set_tacho_ramp_down_sp( dx, 2000 );
 			set_tacho_speed_sp( sn, max_speed * 1 / 6 );
                        set_tacho_speed_sp( dx, max_speed * 1 / 6 );
 			//stabilize the ball
 			Sleep(2000);
			//raise the grabber
			set_tacho_position_sp( med, 90 );
			Sleep(200);
			for ( i = 0; i < 7; i++ ) {
			set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
			Sleep( 200 );
			}
 			get_tacho_position( dx, &act_pos);
 			distance_el=act_pos;
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
			} 
 			Sleep(500);
 			
}   

float go_ahead_till_obstacle(uint8_t sn,uint8_t dx,int max_speed,uint8_t sn_sonar,int distance,uint8_t sn_compass)
{	//aggiungere funzione che controlla anche il motore 
	//sinistro e vede se sono andati dritti tutti e due 
	//altrimenti significa che hai girato e c'è un errore
	//can be used to take the ball once detected 
	//we have to add the angle for the ball a routine to turn till this angle
	//and than go and take te ball
int beginning,finish;
float value;
float initial_angle;
set_tacho_time_sp( sn, 100 );
set_tacho_ramp_up_sp( sn, 2000 );
set_tacho_ramp_down_sp( sn, 2000 );
set_tacho_time_sp( dx, 100 );
set_tacho_ramp_up_sp( dx, 2000 );
set_tacho_ramp_down_sp( dx, 2000 );
get_tacho_position( dx, &beginning);
finish = beginning;
if ( !get_sensor_value0(sn_compass, &initial_angle )) {
                        initial_angle = 0;
                        }
	
while((finish - beginning - distance)<=0){
	control_direction(sn,dx,sn_compass,max_speed,initial_angle);
	set_tacho_time_sp( sn, 100 );
	set_tacho_ramp_up_sp( sn, 2000 );
	set_tacho_ramp_down_sp( sn, 2000 );
	set_tacho_ramp_up_sp( dx, 2000 );
	set_tacho_ramp_down_sp( dx, 2000 );
		
	if ( !get_sensor_value0(sn_sonar, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
			fflush( stdout );
       if(value<2500 && value>=1500)
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
			       }
	if(value<350 && value >=250)
		{
	set_tacho_speed_sp( sn, max_speed * 1 / 6 );
	set_tacho_speed_sp( dx, max_speed * 1 / 6 );
			       }
	if(value<250 && value >=70)
		{
	set_tacho_speed_sp( sn, max_speed * 1 / 12 );
	set_tacho_speed_sp( dx, max_speed * 1 / 12 );
			       }
	if(value<70 && value >=40)
		{
	set_tacho_speed_sp( sn, max_speed * 1 / 24 );
	set_tacho_speed_sp( dx, max_speed * 1 / 24 );
		 }
	if(value<40 && value >=0)
		 {
		printf("sono nello zero\n");
		fflush( stdout );	
	 set_tacho_speed_sp( sn, max_speed * 0 );
	 set_tacho_speed_sp( dx, max_speed * 0 );
	Sleep(100);
	break;
		}
	set_tacho_command_inx( sn, TACHO_RUN_TIMED );
	set_tacho_command_inx( dx, TACHO_RUN_TIMED );
	Sleep(100);
	get_tacho_position( dx, &finish);
}
get_tacho_position( dx, &finish);	
	
	
 return (finish-beginning)/21; //return the distance in cm
}


void movements(uint8_t sn,uint8_t dx,uint8_t sn_sonar, int max_speed, uint8_t sn_compass)
{
	/*	We decide a certain movements to do in order to go from the beginning to the destination and than
		at certain point we scan for the ball.
		Than after we found the ball we can go till the ball, than return back, and restart to follow fixed 
		trajectory, without searching the ball.	
		THE TRAJECTORY CHOSEN IS GO 4 TIMES AHEAD FOR 20 CM 1 TIME AHEAD FOR 10 CM
		THAN TURN LEFT +
	*/
	int i;
	float degree;
	
	int found=0; //this is a flag used to know if the ball has been detected 0=NO 1=YES
	Sleep(5000); //time elapsed to scan
			/*
	if(research(sn, dx, max_speed, sn_compass, 45)==1)
		{	
			found=1;
				//funtion to take the ball and return back}
		else
			*/
			go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,MIN_STEP_VER,sn_compass);
	for(i=0;i<3;i++)
	{
	/*if(found != 1)
		if(research(sn, dx, max_speed, sn_compass, 90)==1)
		{	
			found=1;
				//funtion to take the ball and return back}
		else*/
			go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,MIN_STEP_VER,sn_compass);
	}
	if(found != 1)
		/*
		if(research(sn, dx, max_speed, sn_compass, 90)==1)
		{	
			found=1;
				//funtion to take the ball and return back}
		else*/
			go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,MIN_STEP_VER-10,sn_compass); //here there is -10 becouse we did three step of 20cm	
																			  //than we do another step of 10 cm than we turn left

	//TURN LEFT
	for(i=0;i<3;i++)
	{
	/*if(found != 1)
		if(research(sn, dx, max_speed, sn_compass, 90)==1)
		{	
			found=1;
				//funtion to take the ball and return back}
		else*/
			go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,MIN_STEP_VER,sn_compass);
	}

	//TURN RIGHT
	for(i=0;i<4;i++)
	{
	/*if(found != 1)
		if(research(sn, dx, max_speed, sn_compass, 90)==1)
		{	
			found=1;
				//funtion to take the ball and return back}
		else*/
			go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,MIN_STEP_VER,sn_compass);
	}
	/*if(found != 1)
		if(research(sn, dx, max_speed, sn_compass, 90)==1)
		{	
			found=1;
				//funtion to take the ball and return back}
		else*/
			go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,MIN_STEP_VER-10,sn_compass);	
	//WE HOPE ARRIVED HOME
	get_sensor_value0(sn_compass, &degree);
	
	return;
}

void research(uint8_t sn,uint8_t dx,int max_speed, uint8_t sn_compass, int max_turn_degree)
{	
	float degree;
	float initial;
	rotatedx(sn,dx,sn_compass,max_speed,90);
	
	return;
}
struct motandsens inizialization (struct motandsens donald)
{
  int i;
  
        FLAGS_T state;
        char s[ 256 ];
        int val;
        float value;
        uint32_t n, ii;
	/*
 	printf( "Found tacho motors:\n" );
        for ( i = 0; i < DESC_LIMIT; i++ ) {
                if ( ev3_tacho[ i ].type_inx != TACHO_TYPE__NONE_ ) {
                        printf( "  type = %s\n", ev3_tacho_type( ev3_tacho[ i ].type_inx ));
                        printf( "  port = %s\n", ev3_tacho_port_name( i, s ));
                }
        }*/
	ev3_search_tacho_plugged_in(65,0, &donald.dx, 0 );
	ev3_search_tacho_plugged_in(66,0, &donald.sn, 0 );
	ev3_search_tacho_plugged_in(67,0, &donald.med, 0 );
      if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &donald.sn, 0 )){
                get_tacho_max_speed( donald.sn, &donald.max_speed );
                printf("value of buffer :%d\n", donald.sn);
                printf("  max_speed = %d\n", donald.max_speed );
                set_tacho_stop_action_inx( donald.sn, TACHO_COAST );
                set_tacho_polarity( donald.sn, "inversed" );
                set_tacho_speed_sp( donald.sn, donald.max_speed * 2 / 3 );
                set_tacho_time_sp( donald.sn, 100 );
                set_tacho_ramp_up_sp( donald.sn, 2000 );
                set_tacho_ramp_down_sp( donald.sn, 2000 );
		set_tacho_position( donald.sn,0);
               // set_tacho_command_inx( donald.sn, TACHO_RUN_TIMED );
                /* Wait tacho stop */
                Sleep( 100 );

        } else {
                printf( "LEGO_EV3_L_MOTOR 1 is NOT found\n" );
                fflush( stdout );
        }
//Second motor
if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &donald.dx, 1 )) {


                printf( "LEGO_EV3_L_MOTOR 2 is found, run for 5 sec...\n" );
               
                set_tacho_stop_action_inx( donald.dx, TACHO_COAST );
                set_tacho_polarity( donald.dx, "inversed" );
                set_tacho_speed_sp( donald.dx, donald.max_speed * 2 / 3 );
                set_tacho_time_sp( donald.dx, 100 );
                set_tacho_ramp_up_sp( donald.dx, 2000 );
	        set_tacho_ramp_down_sp( donald.dx, 2000 );
		set_tacho_position( donald.dx,0);
                //set_tacho_command_inx( donald.dx, TACHO_RUN_TIMED );
                /* Wait tacho stop */
                Sleep( 100 );
		fflush( stdout );

        } else {
                printf( "LEGO_EV3_L_MOTOR 2 is NOT found\n" );
		fflush( stdout );
        }
//medium motor
	if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &donald.med, 2 )) {
		int max_speed;

		printf( "LEGO_EV3_L_MOTOR 3 is found, \n" );
		
		set_tacho_stop_action_inx( donald.med, TACHO_COAST );
		set_tacho_polarity( donald.med, "normal" );
		set_tacho_speed_sp( donald.med, donald.max_speed/12);
		set_tacho_time_sp( donald.med, 6000 );
		set_tacho_ramp_up_sp( donald.med, 2000 );
		set_tacho_ramp_down_sp( donald.med, 2000 );
		//set_tacho_command_inx( donald.med, TACHO_RUN_TIMED );
		/* Wait tacho stop */
		Sleep( 100 );
	
	} else {
		printf( "LEGO_EV3_L_MOTOR 1 is NOT found\n" );
		fflush( stdout );
	} 
	
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
        if ( ev3_search_sensor( LEGO_EV3_TOUCH, &donald.sn_touch, 0 )) {
                //printf( "TOUCH sensor is found, press BUTTON for EXIT...\n" );
        }

                if (ev3_search_sensor(HT_NXT_COMPASS, &donald.sn_compass,0)){
                        //printf("COMPASS found, reading compass...\n");
                        if ( !get_sensor_value0(donald.sn_compass, &value )) {
                        value = 0;
                        }
                        printf( "compass\r(%f) \n", value);
                        fflush( stdout );
                }
                if ( ev3_search_sensor( LEGO_EV3_COLOR, &donald.sn_color, 0 )) {
			printf( "COLOR sensor is found, setting...\n" );
			set_sensor_mode( donald.sn_color, "COL-COLOR" );
			if ( !get_sensor_value( 0, donald.sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
				val = 0;
			}
			printf( "\r(%s) \n", color[ val ]);
			printf( "valore del colore e': %d\n",val);
			fflush( stdout );
		}
                if (ev3_search_sensor(LEGO_EV3_US, &donald.sn_sonar,0)){
                        //printf("SONAR found, reading sonar...\n");
                        if ( !get_sensor_value0(donald.sn_sonar, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
			fflush( stdout );
                }
                if (ev3_search_sensor(LEGO_EV3_GYRO, &donald.sn_mag,0)){
                        //printf("GYRO found, reading magnet...\n");
                        if ( !get_sensor_value0(donald.sn_mag, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
                        fflush( stdout );
		}
return donald;
}


	
int main( void )
{
        int i;
	struct motandsens donald;
        //uint8_t sn,dx,med;
        FLAGS_T state;
        //uint8_t sn_touch;
        //uint8_t sn_compass;
        //uint8_t sn_color;
        //uint8_t sn_sonar;
        //uint8_t sn_mag;
        //char s[ 256 ];
        int val;
        //int max_speed;
	int act_pos;
        float value;
	float elapsed_distance;
        //uint32_t n, ii;
	
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

        printf( "*** ( EV3 ) Hello! ***\n" );
	
	donald = inizialization(donald);
	
	/*
        printf( "Found tacho motors:\n" );
        for ( i = 0; i < DESC_LIMIT; i++ ) {
                if ( ev3_tacho[ i ].type_inx != TACHO_TYPE__NONE_ ) {
                        printf( "  type = %s\n", ev3_tacho_type( ev3_tacho[ i ].type_inx ));
                        printf( "  port = %s\n", ev3_tacho_port_name( i, s ));
                }
        }

        if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &sn, 0 )) {


                printf( "LEGO_EV3_L_MOTOR 1 is found, run for 5 sec...\n" );
                get_tacho_max_speed( sn, &max_speed );
                printf("value of buffer :%d\n", sn);
                printf("  max_speed = %d\n", max_speed );
                set_tacho_stop_action_inx( sn, TACHO_COAST );
                set_tacho_polarity( sn, "inversed" );
                set_tacho_speed_sp( sn, max_speed * 2 / 3 );
                set_tacho_time_sp( sn, 100 );
                set_tacho_ramp_up_sp( sn, 2000 );
                set_tacho_ramp_down_sp( sn, 2000 );
		set_tacho_position( sn,0);
              //  set_tacho_command_inx( sn, TACHO_RUN_TIMED );
                // Wait tacho stop 
                Sleep( 100 );
do {
                        get_tacho_state_flags( sn, &state );
                } while ( state );
                //printf( "run to relative position...\n" );
                //set_tacho_speed_sp( sn, max_speed / 2 );
                //set_tacho_ramp_up_sp( sn, 0 );
                //set_tacho_ramp_down_sp( sn, 0 );
                //set_tacho_position_sp( sn, 90 );
                /*for ( i = 0; i < 8; i++ ) {
                        set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
                        Sleep( 500 );
                }
        } else {
                printf( "LEGO_EV3_L_MOTOR 2 is NOT found\n" );
        }
//Second motor
if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &dx, 1 )) {


                printf( "LEGO_EV3_L_MOTOR 2 is found, run for 5 sec...\n" );
                get_tacho_max_speed( dx, &max_speed );
                printf("value of buffer :%d\n", dx);
                printf("  max_speed = %d\n", max_speed );
                set_tacho_stop_action_inx( dx, TACHO_COAST );
                set_tacho_polarity( dx, "inversed" );
                set_tacho_speed_sp( dx, max_speed * 2 / 3 );
                set_tacho_time_sp( dx, 100 );
                set_tacho_ramp_up_sp( dx, 2000 );
	        set_tacho_ramp_down_sp( sn, 2000 );
		set_tacho_position( dx,0);
              //  set_tacho_command_inx( sn, TACHO_RUN_TIMED );
                // Wait tacho stop 
                Sleep( 100 );
		fflush( stdout );
do {
                        get_tacho_state_flags( dx, &state );
                } while ( state );
                //printf( "run to relative position...\n" );
                //set_tacho_speed_sp( sn, max_speed / 2 );
                //set_tacho_ramp_up_sp( sn, 0 );
                //set_tacho_ramp_down_sp( sn, 0 );
                //set_tacho_position_sp( sn, 90 );
                /*for ( i = 0; i < 8; i++ ) {
                        set_tacho_command_inx( sn, TACHO_RUN_TO_REL_POS );
                        Sleep( 500 );
                }
        } else {
                printf( "LEGO_EV3_L_MOTOR 2 is NOT found\n" );
		fflush( stdout );
        }
//medium motor
	if ( ev3_search_tacho( LEGO_EV3_L_MOTOR, &med, 2 )) {
		int max_speed;

		printf( "LEGO_EV3_L_MOTOR 1 is found, run for 5 sec...\n" );
		get_tacho_max_speed( med, &max_speed );
		printf("  max_speed = %d\n", max_speed );
		set_tacho_stop_action_inx( med, TACHO_COAST );
		set_tacho_polarity( med, "normal" );
		set_tacho_speed_sp( med, max_speed);
		set_tacho_time_sp( med, 6000 );
		set_tacho_ramp_up_sp( med, 2000 );
		set_tacho_ramp_down_sp( med, 2000 );
		//set_tacho_command_inx( med, TACHO_RUN_TIMED );
		// Wait tacho stop 
		Sleep( 100 );
		do {
			get_tacho_state_flags( sn, &state );
		} while ( state );
		printf( "run to relative position...\n" );
		set_tacho_speed_sp( med, max_speed/12);
		set_tacho_ramp_up_sp( med, 0 );
		set_tacho_ramp_down_sp( med, 0 );
		set_tacho_position_sp( med, 20 );
		for ( i = 0; i < 7; i++ ) {
			set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
			Sleep( 200 );
		}
		Sleep( 200 );
		fflush( stdout );
	} else {
		printf( "LEGO_EV3_L_MOTOR 1 is NOT found\n" );
		fflush( stdout );
	} 
	//set_tacho_polarity( med, "inversed" );
	//set_tacho_stop_action_inx(med,TACHO_HOLD);
	//set_tacho_hold_pid_Kd 	(med,1000); 
	//ev3_stop_command_motor(med,2);
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
        if ( ev3_search_sensor( LEGO_EV3_TOUCH, &sn_touch, 0 )) {
                //printf( "TOUCH sensor is found, press BUTTON for EXIT...\n" );
        }
*/
   
        while(1){
		/*
                if (ev3_search_sensor(HT_NXT_COMPASS, &sn_compass,0)){
                        //printf("COMPASS found, reading compass...\n");
                        if ( !get_sensor_value0(sn_compass, &value )) {
                        value = 0;
                        }
                        printf( "compass\r(%f) \n", value);
                        fflush( stdout );
                }
                if ( ev3_search_sensor( LEGO_EV3_COLOR, &sn_color, 0 )) {
			//printf( "COLOR sensor is found, reading COLOR...\n" );
			set_sensor_mode( sn_color, "COL-COLOR" );
			if ( !get_sensor_value( 0, sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
				val = 0;
			}
			printf( "\r(%s) \n", color[ val ]);
			printf( "valore del colore e': %d\n",val);
			fflush( stdout );
		}
                if (ev3_search_sensor(LEGO_EV3_US, &sn_sonar,0)){
                        //printf("SONAR found, reading sonar...\n");
                        if ( !get_sensor_value0(sn_sonar, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
			fflush( stdout );
                }
                if (ev3_search_sensor(LEGO_EV3_GYRO, &sn_mag,0)){
                        //printf("GYRO found, reading magnet...\n");
                        if ( !get_sensor_value0(sn_mag, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
                        fflush( stdout );
                }
	//research( sn, dx, max_speed, sn_compass);
	//break;
         */    
	elapsed_distance = go_ahead_till_obstacle(donald.sn,donald.dx,donald.max_speed,donald.sn_sonar,2000,donald.sn_compass);

	
	Sleep(2000);
	fflush( stdout );
		/*
	if( strcmp(color[ color_aq(sn_color) ],"RED")==0)
	grab_ball(sn,dx,med,max_speed);
	//if( strcmp(color[ val ],"RED")==0)
	else
	rotatedx(sn,dx,sn_compass,max_speed,45);
	//leave_ball(sn,dx,med,max_speed);
	*/
	/*
        rotatesx(sn,dx,sn_compass,max_speed,90);
	break;*/

               
		
        }
		
		
		/*	
	
	rotatedx(sn,dx,max_speed);
	rotatesx(sn,dx,max_speed);
	*/
		
        ev3_uninit();
        printf( "*** ( EV3 ) Bye! ***\n" );

        return ( 0 );
}



