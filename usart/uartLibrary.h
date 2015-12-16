/**
 * This library contains the interaction with the UART interface.
 * 
 * Usage:
 * 	1) In your program include "uartLibrary.h"
 * 	2) In your main function, redefine stdout and stdin:
 * 	   		stdout = &uart_output;
 * 	   		stdin  = &uart_input;
 * 	3) If you successfully implemented the functions below,
 * 	   you will be able to use
 * 	   		puts(string s);
 * 	   		printf(string, parameters);
 * 	   to print data into the UART interface. With
 * 	   		char c = getChar();
 *     you can read from the terminal.
 *
 * For further information have a look at the example code that is
 * included.
 */


#ifndef UART_LIBRARY_INCLUDED
#define UART_LIBRARY_INCLUDED

#define FOSC 16000000UL // Clock Speed
#define BAUD 9600
#define MYUBRR (FOSC/16/BAUD-1)

#include <util/setbaud.h>
#include <avr/io.h>
#include <stdio.h>

/**
 * Configures the UART port
 * Put all parameters that need to be set regarding the UART here.
 */
void uart_init() {
	/*
	UCSRA0A -> [ RXC:0 TXC:0 UDRE:0 FE:0 DOR:0 UPE:0 U2x:0 MPCM:0 ]
	*/
	UCSR0A |= 0x00;
	/*
	UCSR0B -> [ RXCIE:1 TXCIE:0 UDRIE:0 RXEN:1 TXEN:1 UCSZ02:0 RXB80:0 TXB80:0 ]
	*/
	UCSR0B |= 0b10011000;
	/*
	UCSR0C -> [ UMSEL00:1=00 UPM01:0=00 USBS0:0 UCSZ01:1 UCSZ00:1 UCPOL0:0 ]
	*/
	UCSR0C |= 0b00000110;

	UBRR0H = MYUBRR >> 8;
	UBRR0L = MYUBRR;
}
	

/**
 * Prints a character to the UART interface
 * @params c: the character to be printed
 * @params *stream: The file stream. Do not worry what
 * 		    is put there. It is used internally by puts
 * 		    or printf.
 */
void uart_putchar(char c, FILE *stream) {
	if (c == '\n') {		// format the string nicely for display on the serial terminal
		uart_putchar('\r', stream);
	}

	// Wait till Data Buffer is Empty
	while (1)
	{
		if (UCSR0A & 0b00100000)	// check UDRE0
			break;
	}
	UDR0 = c;
}


/**
 * Get a character from the UART interface
 * @params *stream: The file stream. Do not worry what
 * 		    is put there. It is used internally by puts
 * 		    or printf.
 */
char uart_getchar(FILE *stream) {
	// Wait till Data is available in Data Buffer
	while (1)
	{
		if (UCSR0A & 0b10000000)	// check RXC0
			break;
	}
	return UDR0;
}


/**
 * Allows you to use printf, puts as commands to print something through the UART
 * on your serial console. It then can be displayed using HyperTerminal, minicom etc.
 */
FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

/**
 * Allows you to use getChar() to get a character from the user through UART
 */
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);


#endif

