/**
 * Bootloader um dem Mikrocontroller Bootloader von Peter Dannegger anzusteuern
 *
 * License: GPL
 *
 * @author Andreas Butti
 */

#ifndef COM_H_INCLUDED
#define COM_H_INCLUDED

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// Baudrate definitions

#define BAUD_CNT 18

extern speed_t baud_const[BAUD_CNT];
extern unsigned long baud_value[BAUD_CNT];
extern unsigned int crc;

/// Prototypes

/**
 * Set flag for one-wire local echo
 *
 */
void com_localecho ();

/**
 * Opens com port
 *
 * @return true if successfull
 */
char com_open(const char * device, speed_t baud);

/**
 * Close com port and restore settings
 */
void com_close();

/**
 * Sends one char
 */
void com_putc_fast(unsigned char c);
void com_putc(unsigned char c);

/**
 * Recives one char or -1 if timeout
 */
int com_getc(int timeout);

/**
 * Flushes the buffer
 */
void com_flush();

/**
 * Sends a string
 */
void com_puts(const char *text);

/**
 * Sending a command
 */
void sendcommand(unsigned char c);

#endif //COM_H_INCLUDED
