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
#include "commonFunctions.h"
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
  
  /*---------------------------------------------------------------compass----------------------------------------------------------*/
  float get_compass_values(uint8_t sn_compass) {
	int i;
	float sum;
	float degree;
	sum=0;
	for(i=0;i<10;i++)
	{
		
		get_sensor_value0(sn_compass, &degree);
		if ( !get_sensor_value0(sn_compass, &degree )) 
			{
			   degree = 0;
			} 
		Sleep(100);
		sum+=degree;
	}
	return sum/10;	
	
}

/*---------------------------------------------------------------position----------------------------------------------------------*/
  void* position(void *args) {
   int flag_rot; 
    struct motandsens *donald = (struct motandsens *) args;
    int retour;
    float degree, first_comp;
    int motor_value,iniz_comp,res_cond;
    float x_new,y_new,x_old,y_old,x_start,y_start,x_lim,y_lim;
/////////////////////////do a switch case of if based on where you are ( small or big arena, side) and rol\\\\\
//////thread
//	retour = pthread_mutex_lock(&mutex);
//	 retour = pthread_mutex_lock(&mutex);
  //  			if (retour != 0)
    //			 {
    //			   perror("erreur mutex lock");
     //			  exit(EXIT_FAILURE);
    	//		 }
/////////	
	flag_rot=0;
	switch (donald->arena)
	{
       case 0 :
         switch (donald->role)
         {
       		case 0 :
       		  x_start=90;
        	  y_start=25;
        	  iniz_comp=0;
              break;
    		case 1 :
    		  x_start=30;
        	  y_start=175;
        	  iniz_comp=180;
     		  break;

         }
         break;
       case 1 :
         switch (donald->side)
         {
 			case 0 :
 			   switch (donald->role)
               {
       		      case 0 :
       		         x_start=30;
        	         y_start=25;
        	         iniz_comp=0;
     		         break;
    		      case 1 :
    		         x_start=90;
        	         y_start=375;
        	         iniz_comp=180;
        	         break;
               }
                break;
            case 1:
 			   switch (donald->role)
               {
       		      case 0 :
       		         x_start=-30;
        	         y_start=25;
        	         iniz_comp=0;
                     break;
    		      case 1 :
    		         x_start=-90;
        	         y_start=375;
        	         iniz_comp=180;
       		         break;
               }
                break;

         }
         break;
    } 
 //////////////////////////////////////////
   int  i=0;
    res_cond=0;
     while ( res_cond==0 )
     {
        get_tacho_position(donald->sn,&motor_value);
        /*
	if ( !get_sensor_value0(donald->sn_compass, &degree )) {
                        degree = 0;
                        }   */
	degree = get_compass_values(donald->sn_compass);
        if(i==0)
        {   
            first_comp=degree;
		
  			x_new=x_start+motor_value/21*sin(M_PI/180*(degree-first_comp+iniz_comp));
  			y_new=y_start+motor_value/21*cos(M_PI/180*(degree-first_comp+iniz_comp));
  			i=1;
  			donald->x=x_new;
  			donald->y=y_new;
  			//send realpos.x and .y/
  			printf( "x=%f  y=%f \n" ,donald->x, donald->y);
  			
        }
        else
        {    
   			x_old=x_new;
   			y_old=y_new;
   			if(flag_rot==0)   /*go haed*/
   			{       printf("sono nell'if cond, degree:%f , firstcomp:%f, diff:%d ", degree,first_comp,degree-first_comp);
   				x_new=x_old+(motor_value/21-x_old)*sin(M_PI/180*(degree-first_comp+iniz_comp));
   				y_new=y_old+(motor_value/21-y_old)*cos(M_PI/180*(degree-first_comp+iniz_comp));
   				donald->x=x_new;
  			        donald->y=y_new;
				//send realpos.x and .y/
				printf( "x=%f  y=%f \n" , donald->x, donald->y);
  				
			}
			else   /*turning*/
			{
 				//x_old=x_new;
   				//y_old=y_new;
   				x_new=motor_value/21*sin(M_PI/180*(degree-first_comp+iniz_comp));
   				y_new=motor_value/21*cos(M_PI/180*(degree-first_comp+iniz_comp));
   				donald->x=x_old;
  				donald->y=y_old;
  				//send realpos.x and .y/    /setta flag sulla fifo s per dire che il robot può girare/
  				printf( "x=%f  y=%f \n" , donald->x, donald->y);
  				
	               	}
	
        }
	     Sleep(2000);
    	res_cond=getcondition(0,0,0,donald->x,donald->y);
  printf(" res_cond=%d\n" , res_cond); 
        //gira se sei arrivato ai limiti dell'arena/
        //void dont_pass_arena_limits (realpox.x,realpos.y, sn, dx, sn_compass, max_speed,role,side,arena,first_comp,degree)
     }
   //send start to the finisher or stop if you are the finisher/
	//trhead
		
 		//	 retour = pthread_mutex_unlock(&mutex);
    		//	if (retour != 0)
    		//	 {
    		//	   perror("erreur mutex unlock");
     		//	  exit(EXIT_FAILURE);
    		//	 }
}



/*------------------------------------------------------------TURN RIGHT-------------------------------------------------------------*/
  void rotatedx(uint8_t sn, uint8_t dx, uint8_t sn_compass, int max_speed, int rotation, uint8_t sn_mag) {	
  float actual_angle;
	float wanted_c;
	set_tacho_position( sn,0);
	set_tacho_position( dx,0);
	set_tacho_speed_sp( sn, max_speed/6);
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_speed_sp( dx, max_speed/6);
	set_tacho_ramp_up_sp( dx, 0 );
	set_tacho_ramp_down_sp( dx, 0 );
	set_tacho_position_sp( sn, 2 );
	set_tacho_position_sp( dx, -2);
	if ( !get_sensor_value0(sn_mag, &actual_angle )) {
                        actual_angle = 0;
		}
	wanted_c= actual_angle + rotation;

	while((actual_angle-wanted_c)<0)
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

/*------------------------------------------------------------TURN LEFT-------------------------------------------------------------*/
  void rotatesx(uint8_t sn, uint8_t dx, uint8_t sn_compass, int max_speed, int rotation, uint8_t sn_mag);
  
  
  {	float actual_angle;
	float wanted_c;
	set_tacho_position( sn,0);
	set_tacho_position( dx,0);
	set_tacho_speed_sp( sn, max_speed/6);
	set_tacho_ramp_up_sp( sn, 0 );
	set_tacho_ramp_down_sp( sn, 0 );
	set_tacho_speed_sp( dx, max_speed/6);
	set_tacho_ramp_up_sp( dx, 0 );
	set_tacho_ramp_down_sp( dx, 0 );
	set_tacho_position_sp( sn,  -2);
	set_tacho_position_sp( dx, 2);
	if ( !get_sensor_value0(sn_mag, &actual_angle )) {
                        actual_angle = 0;
		}
	wanted_c= actual_angle - rotation;

	while((actual_angle-wanted_c)>0)
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
  
  
  /*------------------------------------------------Keep a straight line-------------------------------------------------------------*/
  void control_direction(uint8_t sn,uint8_t dx,uint8_t sn_compass,int max_speed, float initial_angle,uint8_t sn_mag){
		int minsize;
		float actual_angle;
		minsize=1;
		if ( !get_sensor_value0(sn_mag, &actual_angle)) {
			actual_angle = 0;
		}
		//printf("initial  %f\n",initial_angle);
		//printf("final %f\n", actual_angle);
		if(actual_angle!=initial_angle)
		{	
			if(actual_angle<(initial_angle - 4))	//too to the left turn right!!!
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
					if ( !get_sensor_value0(sn_mag, &actual_angle)){
					actual_angle=0;
					}
				
				}
			
			}
			if(actual_angle> (initial_angle + 4))	//too to the right turn left!!!
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
					 if ( !get_sensor_value0(sn_mag, &actual_angle)){
                                        actual_angle=0;
                                        }
				}
			
			}
			
		}
}
  /*------------------------------------------------------Grab the ball-------------------------------------------------------------*/
  void grab_ball(uint8_t sn,uint8_t dx,uint8_t med,int max_speed){			
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
			for ( i = 0; i < 5; i++ ) {
			set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
			Sleep( 200 );
			}
 			get_tacho_position( dx, &act_pos);
 			distance_el=act_pos;
 			while((act_pos-(8*26)-distance_el)<=0)
			{
				set_tacho_command_inx( sn, TACHO_RUN_TIMED );
				set_tacho_command_inx( dx, TACHO_RUN_TIMED );
				get_tacho_position( dx, &act_pos);
			}
			//release the grabber
			set_tacho_position_sp( med, -80 );
			Sleep(200);
			for ( i = 0; i < 1; i++ ) {
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
  
  
  /*------------------------------------------------------------Color sensor-------------------------------------------------------------*/
  int color_aq(uint8_t sn_color) {	
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

  
  /*--------------------------------------------Leave the ball and stay up-------------------------------------------------------------*/
  void leave_ball(uint8_t sn,uint8_t dx,uint8_t med,int max_speed){			
      int i;
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
  /*---------------------------------------Put down, should be used after living the ball-------------------------------------------*/
  void put_down(uint8_t sn,uint8_t dx,uint8_t med,int max_speed){
      int i;
			int act_pos,distance_el;
			//stabilize the ball
 			set_tacho_position_sp( med, -90 );
			Sleep(200);
			for ( i = 0; i < 4; i++ ) {
			set_tacho_command_inx( med, TACHO_RUN_TO_REL_POS );
			Sleep( 200 );
			} 
 			Sleep(500);
 			
} 
  
 
  
  /*------------------------------------------------------------Go backward-------------------------------------------------------*/
  void go_backward(uint8_t sn,uint8_t dx,uint8_t med,int max_speed){	
      int i;
			int act_pos,distance_el;
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
 			printf("sono qui in vai indietro");
 			for(i=0;i<6;i++){
 			set_tacho_command_inx( sn, TACHO_RUN_TIMED );
			set_tacho_command_inx( dx, TACHO_RUN_TIMED );
 			Sleep(500);
			}
			get_tacho_position( dx, &act_pos);
 			Sleep(500);
 			
}  
  
  /*----------------------go aheah with different speeds depending on if an obstacle is detected near by or not---------------------*/
  float go_ahead_till_obstacle(uint8_t sn,uint8_t dx,int max_speed,uint8_t sn_sonar,int distance,uint8_t sn_compass, uint8_t sn_mag) {	
  //aggiungere funzione che controlla anche il motore 
	//sinistro e vede se sono andati dritti tutti e due 
	//altrimenti significa che hai girato e c'è un errore
	//can be used to take the ball once detected 
	//we have to add the angle for the ball a routine to turn till this angle
	//and than go and take te ball
int beginning,finish,partial;
int retour;
float value;
float initial_angle;
set_tacho_time_sp( sn, 100 );
set_tacho_ramp_up_sp( sn, 1000 );
set_tacho_ramp_down_sp( sn, 1000);
set_tacho_time_sp( dx, 100 );
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
	set_tacho_time_sp( sn, 100 );
	set_tacho_ramp_up_sp( sn, 1000 );
	set_tacho_ramp_down_sp( sn, 1000 );
	set_tacho_time_sp( dx, 100);
	set_tacho_ramp_up_sp( dx, 1000 );
	set_tacho_ramp_down_sp( dx, 1000 );
	retour = pthread_mutex_lock(&mutex);
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
	set_tacho_speed_sp( sn, max_speed * 1 / 8 );
	set_tacho_speed_sp( dx, max_speed * 1 / 8 );
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

 /*------------------------------------------------------color sensor-------------------------------------------------*/
void* colorsense(void * args){ 	
  int val;
 	struct motandsens *donald = (struct motandsens *) args;
 	while(1)
	{
 
	if ( !get_sensor_value( 0, donald->sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
				val = 0;
			}
			if( strcmp(color[ color_aq(donald->sn_color) ],"RED")==0)
			{
				grab_ball(donald->sn,donald->dx,donald->med,donald->max_speed);
				Sleep(200);
				
			}
	}
	return;
}
  
    /*------------------------------------------------------initialize motors and sensors------------------------------------------------*/
   struct motandsens* inizialization (struct motandsens *donald){
int i;
FLAGS_T state;
char s[ 256 ];
int val;
int max_speed;
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
return donald;
}
   
   
     /*------------------------------------------------------search for the ball---------------------------------------------------*/
   void research(uint8_t sn,uint8_t dx,int max_speed, uint8_t sn_compass, int max_turn_degree, uint8_t med, uint8_t sn_mag, uint8_t sn_sonar){
   //Take the initial position than move to 
float initial_angle;
float actual_angle;
float elapsed_dis;
float start_angle, final_angle, middle_angle;
int pos_in_sn, pos_in_dx, pos_in_ball_sn, pos_in_ball_dx; 
int pos_fin_ball_sn, pos_fin_ball_dx, found_sn, found_dx;
int i, k, flag_1;
int points[1000]={0};
if ( !get_sensor_value0(sn_mag, &initial_angle )) 
   {
   initial_angle = 0;
   }
get_tacho_position(sn, &pos_in_sn);
get_tacho_position(dx, &pos_in_dx);
flag_1=0;

//turn right 45 ° and start moving 2° each step
rotatedx(sn,dx,sn_compass,max_speed,45,sn_mag);	
for(i=0;i<45;i++)
{
printf("I'M here\n");
Sleep(500);
get_sensor_value0(sn_sonar, &points[i]);
printf("Il valore è %d:\n",points[i]);
if(i!=0 && ((points[i-1]-points[i])>=350) && flag_1==0)
{
 //this is the first balls' extremity 
	if ( !get_sensor_value0(sn_mag, &start_angle )) 
	{
	start_angle = 0;
	}
	get_tacho_position(sn,&pos_in_ball_sn);
	get_tacho_position(dx,&pos_in_ball_dx);
	flag_1=1;
}
if(i!=0 && ((points[i]-points[i-1])>=350) && flag_1==1)
{
  //this is the last point of the ball detected
	if ( !get_sensor_value0(sn_mag, &final_angle )) 
	{
	final_angle = 0;
	}
	get_tacho_position(sn,&pos_in_ball_sn);
	get_tacho_position(dx,&pos_in_ball_dx);
	flag_1=2;

}
if(flag_1==2)
{	
	middle_angle = (final_angle - start_angle) / 2;
	found_sn=(pos_fin_ball_sn - pos_in_ball_sn) / 2;
	found_dx=(pos_fin_ball_dx - pos_in_ball_dx) / 2;
	break;
}
rotatesx(sn,dx,sn_compass,max_speed,2,sn_mag);
Sleep(100);
}
//it has finished the search
//restart from centre and go to the desired angle
rotatedx(sn,dx,sn_compass,max_speed,90,sn_mag); 
rotatesx(sn,dx,sn_compass,max_speed,middle_angle,sn_mag);
elapsed_dis=go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,4000,sn_compass,sn_mag);
rotatedx(sn,dx,sn_compass,max_speed,180,sn_mag);
go_ahead_till_obstacle(sn,dx,max_speed,sn_sonar,elapsed_dis,sn_compass,sn_mag);
rotatesx(sn,dx,sn_compass,max_speed,180,sn_mag);
rotatedx(sn,dx,sn_compass,max_speed,middle_angle,sn_mag);
//hope it will work=)
return;
}
