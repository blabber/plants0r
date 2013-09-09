/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

/**
 * Hardware USART
 *
 * 8N1 @ 9600bd
 */

#define BAUD	9600

#include <util/setbaud.h>

#include "usart.h"

#define TRANSMIT_BUFFER_IS_EMPTY( )	(UCSR0A & (1<<UDRE0))
#define TRANSMIT_BUFFER			UDR0
#define SET_TRANSMIT_MODE_8N1( )	(UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00))
#define SET_BAUD_RATE( )		do {				\
						UBRR0H = UBRRH_VALUE; 	\
						UBRR0L = UBRRL_VALUE;	\
					} while (0)
#define ENABLE_TX( )			(UCSR0B |= (1<<TXEN0))

void
UA_init(void)
{
	ENABLE_TX();
	SET_TRANSMIT_MODE_8N1();
	SET_BAUD_RATE();
}

void
UA_putc(char *c)
{
	while (!TRANSMIT_BUFFER_IS_EMPTY())
		/* nop */;

	TRANSMIT_BUFFER = *c;
}

void
UA_puts(char *s)
{
	char *l = s;

	while (*l != '\0')
		UA_putc(l++);
}
