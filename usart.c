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

#include <stdint.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

#include "usart.h"
#include "buffer.h"

#define TX_BUFFER			UDR0
#define IS_TX_BUFFER_READY( )		(UCSR0A & (1<<UDRE0))
#define SET_TRANSMIT_MODE_8N1( )	(UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00))
#define SET_BAUD_RATE( )		do {				\
						UBRR0H = UBRRH_VALUE; 	\
						UBRR0L = UBRRL_VALUE;	\
					} while (0)
#define ENABLE_TX( )			(UCSR0B |= (1<<TXEN0))
#define ENABLE_TX_COMPLETE_IRQ( )	(UCSR0B |= (1<<TXCIE0))

#define BUFFLEN	32

struct buffer *buff;

void
UA_init(void)
{
	ENABLE_TX();
	SET_TRANSMIT_MODE_8N1();
	SET_BAUD_RATE();
	ENABLE_TX_COMPLETE_IRQ();

	buff = buf_create(BUFFLEN);
}

void
UA_putc(uint8_t c)
{
	while (buf_putc(buff, c) != 0);

	if (IS_TX_BUFFER_READY()) {
		if (buf_getc(buff, &c) == 0)
			TX_BUFFER = c;
	}
}

void
UA_puts(char *s)
{
	char *l = s;

	while (*l != '\0')
		UA_putc(*(l++));
}

ISR(USART_TX_vect)
{
	uint8_t c;
	if (buf_getc(buff, &c) == 0)
		TX_BUFFER = c;
}
