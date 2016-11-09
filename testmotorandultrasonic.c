#include <stdio.h>

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
int main( void )
{
        int i;
        uint8_t sn,dx,med;
        FLAGS_T state;
        uint8_t sn_touch;
        uint8_t sn_compass;
        uint8_t sn_color;
        uint8_t sn_sonar;
        uint8_t sn_mag;
        char s[ 256 ];
        int val;
        int max_speed;
        float value;
        uint32_t n, ii;
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
              //  set_tacho_ramp_down_sp( sn, 2000 );
              //  set_tacho_command_inx( sn, TACHO_RUN_TIMED );
                /* Wait tacho stop */
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
                }*/
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
              //  set_tacho_ramp_down_sp( sn, 2000 );
              //  set_tacho_command_inx( sn, TACHO_RUN_TIMED );
                /* Wait tacho stop */
                Sleep( 100 );
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
                }*/
        } else {
                printf( "LEGO_EV3_L_MOTOR 2 is NOT found\n" );
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
		/* Wait tacho stop */
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
			Sleep( 150 );
		}
	} else {
		printf( "LEGO_EV3_L_MOTOR 1 is NOT found\n" );
	} 
	set_tacho_stop_action_inx( med, HOLD );

//Run all sensors
        ev3_sensor_init();

        printf( "Found sensors:\n" );
        for ( i = 0; i < DESC_LIMIT; i++ ) {
                if ( ev3_sensor[ i ].type_inx != SENSOR_TYPE__NONE_ ) {
                        printf( "  type = %s\n", ev3_sensor_type( ev3_sensor[ i ].type_inx ));
                        printf( "  port = %s\n", ev3_sensor_port_name( i, s ));
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
                printf( "TOUCH sensor is found, press BUTTON for EXIT...\n" );
        }
        while(1){

                if (ev3_search_sensor(HT_NXT_COMPASS, &sn_compass,0)){
                        printf("COMPASS found, reading compass...\n");
                        if ( !get_sensor_value0(sn_compass, &value )) {
                        value = 0;
                        }
                        printf( "\r(%f) \n", value);
                        fflush( stdout );
                }
                if ( ev3_search_sensor( LEGO_EV3_COLOR, &sn_color, 0 )) {
			printf( "COLOR sensor is found, reading COLOR...\n" );
			if ( !get_sensor_value( 0, sn_color, &val ) || ( val < 0 ) || ( val >= COLOR_COUNT )) {
				val = 0;
			}
			printf( "\r(%s) \n", color[ val ]);
			fflush( stdout );
		}
                if (ev3_search_sensor(LEGO_EV3_US, &sn_sonar,0)){
                        printf("SONAR found, reading sonar...\n");
                        if ( !get_sensor_value0(sn_sonar, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
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
                                if(value<500 && value >=150)
                                        {
                                set_tacho_speed_sp( sn, max_speed * 1 / 3 );
                                set_tacho_speed_sp( dx, max_speed * 1 / 3 );
                                                       }
                                if(value<150 && value >=70)
                                        {
                                set_tacho_speed_sp( sn, max_speed * 1 / 6 );
                                set_tacho_speed_sp( dx, max_speed * 1 / 6 );
                                         }
                                if(value<70 && value >=0)
                                         {       
                                set_tacho_speed_sp( sn, max_speed * 0 );
                                 set_tacho_speed_sp( dx, max_speed * 0 );
                                                 }
                                set_tacho_command_inx( sn, TACHO_RUN_TIMED );
                                       set_tacho_command_inx( dx, TACHO_RUN_TIMED );



                        fflush( stdout );
                }
                if (ev3_search_sensor(LEGO_EV3_GYRO, &sn_mag,0)){
                        printf("GYRO found, reading magnet...\n");
                        if ( !get_sensor_value0(sn_mag, &value )) {
                                value = 0;
                        }
                        printf( "\r(%f) \n", value);
                        fflush( stdout );
                }


        }

        ev3_uninit();
        printf( "*** ( EV3 ) Bye! ***\n" );

        return ( 0 );
}




