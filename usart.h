/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#ifndef _USART_H_
#define _USART_H_

#include <avr/io.h>

#define UA_BAUD		9600
#define UA_BUFFLEN	32

#define UA_TX_BUFF			UDR0
#define UA_SET_TRANSMIT_MODE_8N1( )	(UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00))
#define UA_ENABLE_TX( )			(UCSR0B |= (1<<TXEN0))
#define UA_ENABLE_TX_BUFF_READY_IRQ( )	(UCSR0B |= (1<<UDRIE0))
#define UA_DISABLE_TX_BUFF_READY_IRQ( )	(UCSR0B &= ~(1<<UDRIE0))
#define UA_SET_BAUD_RATE( )		do {                            \
						UBRR0H = UBRRH_VALUE;   \
						UBRR0L = UBRRL_VALUE;   \
					} while (0)
#define UA_TX_BUFF_READY_VECTOR		USART_UDRE_vect

void UA_init(void);
void UA_putc(uint8_t c);
void UA_puts(char *s);

#endif
