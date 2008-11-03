/**
 * Bootloader um dem Mikrocontroller Bootloader von Peter Dannegger anzusteuern
 *
 * License: GPL
 *
 * @author Andreas Butti
 */


/// Includes
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/times.h>

#include "com.h"
#include "protocol.h"


/// Attributes

// Device
int fd;
// Old settings
struct termios oldtio;
// CRC checksum
unsigned int crc = 0;

int sendCount = 0;

/// Prototypes
void calc_crc(unsigned char d);


speed_t baud_const[BAUD_CNT] = { B50, B75, B110, B134, B150, B200, B300, B600,
                                 B1200, B1800, B2400, B4800, B9600, B19200, 
                                 B38400, B57600, B115200, B230400 };

unsigned long baud_value[BAUD_CNT] = { 50, 75, 110, 134, 150, 200, 300, 600,
                                       1200, 1800, 2400, 4800, 9600, 19200,
                                       38400, 57600, 115200, 230400 };


/**
 * Set flag for one-wire local echo
 *
 */
void com_localecho ()
{
    sendCount = 1;
}


/**
 * Opens com port
 *
 * @return true if successfull
 */
char com_open(const char * device, speed_t baud)
{
    struct termios newtio;

    // Open the device
    fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        return 0;
    }

    // Save old settings
    tcgetattr(fd, &oldtio);

    // Init memory
    memset(&newtio, 0x00 , sizeof(newtio));

    // settings flags
    newtio.c_cflag = CS8 | CLOCAL | CREAD; // | CSTOPB;
    newtio.c_iflag = IGNPAR | IGNBRK;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;

    cfmakeraw(&newtio);

    // Timeout in 100ms
    newtio.c_cc[VTIME] = 0;
    // read 1 character
    newtio.c_cc[VMIN] = 0;

    // Setting baudrate
    cfsetispeed(&newtio, baud);

    // Flushing buffer
    tcflush(fd, TCIOFLUSH);

    // aplying new configuration
    tcsetattr(fd, TCSANOW, &newtio);

    sendCount = 0;

    return 1;
}


/**
 * Close com port and restore settings
 */
void com_close() 
{
    // restore old settings
    tcsetattr(fd, TCSANOW, &oldtio);

    // close device
    close(fd);
}

/**
 * Receives one char or -1 if timeout
 * timeout in 10th of seconds
 */
int com_getc(int timeout) 
{
    static long         ticks = 0;
    static struct tms   theTimes;
    char    c;
    clock_t t = times (&theTimes);

    if (ticks == 0)
        ticks = sysconf(_SC_CLK_TCK) / 10;

    do 
    {
        if (read(fd, &c, 1))
        {
//            printf (" read: %c %02X", c, c & 0x00ff);
            if (sendCount > 1)
            {
                sendCount--;
                t = times (&theTimes);
                continue;
            }
//            printf ("\n");
            return (unsigned char)c;
        }
    } while ( ((times (&theTimes) - t )/ticks) < timeout );

    return -1;
}

/**
 * Sends one char
 */
void com_putc_fast(unsigned char c)
{
    char a;

#if 1
    if (sendCount)
    {
        if (sendCount > 1)
            com_getc(0);
        sendCount++;
    }
#else
    if (sendCount > 0)
    {
        if (read(fd, &a, 1))
            sendCount--;
//        com_getc(0);
        sendCount++;
    }
#endif
    write(fd, &c, 1);
//    printf ("Snd: '%c' %02X\n", c, c);
    calc_crc( c ); // calculate transmit CRC
}

void com_putc(unsigned char c) 
{
    tcdrain(fd);
    com_putc_fast (c);
}


/**
 * Sending a command
 */
void sendcommand(unsigned char c)
{
    if (sendCount)
        sendCount = 1;
    com_putc(COMMAND);
    com_putc(c);
}


/**
 * Flushes the buffer
 */
void com_flush() 
{
    // Flushing buffer
    tcflush(fd, TCIOFLUSH);
}

/**
 * Sends a string
 */
void com_puts(const char *text) 
{
    while(*text) 
    {
        com_putc( *text++);
    }
}

/**
 * Calculate the new CRC sum
 */
void calc_crc(unsigned char d) 
{
    int i;

    crc ^= d;
    for( i = 8; i; i-- )
    {
        crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0 );
    }
}

