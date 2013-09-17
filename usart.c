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

struct buffer *buff;

void
UA_init(void)
{
	UA_ENABLE_TX();
	UA_SET_TRANSMIT_MODE_8N1();
	UA_SET_BAUD_RATE();

	buff = buf_create(UA_BUFFLEN);
}

void
UA_putc(uint8_t c)
{
	while (buf_putc(buff, c) != 0);

	UA_ENABLE_TX_BUFF_READY_IRQ();
}

void
UA_puts(char *s)
{
	char *l = s;
	while (*l != '\0')
		UA_putc(*(l++));
}

ISR(USART_UDRE_vect)
{
	uint8_t c;
	if (buf_getc(buff, &c) == 0)
		UA_TX_BUFF = c;
	else
		UA_DISABLE_TX_BUFF_READY_IRQ();
}
