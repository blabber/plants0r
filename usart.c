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

#define TRANSMIT_BUFFER_IS_EMPTY( )	(UCSRA & (1<<UDRE))
#define TRANSMIT_BUFFER			UDR
#define SET_TRANSMIT_MODE_8N1( )	(UCSRC |= (1<<UCSZ1) | (1<<UCSZ0))
#define SET_BAUD_RATE( )		do {				\
						UBRRH = UBRRH_VALUE; 	\
						UBRRL = UBRRL_VALUE;	\
					} while (0)
#define ENABLE_TX( )			(UCSRB |= (1<<TXEN))

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
