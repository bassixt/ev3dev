#include "bt_messages.h"
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
       // int role;/*0 beg 1 fin*/
       // int arena;/*0 small1 big*/
       //int side;/*0 right 1 left*/
	int number;	   


};


//READFROMSERVER
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
//SENDPOSITION
void send_pos_to_bt (void* args)
{         
        char string[58];
        int8_t x_conv_MSB,x_conv_LSB,y_conv_MSB,y_conv_LSB;
        struct motandsens *donald = (struct motandsens *) args; 
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
}
