#include "btmessages.h"


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
