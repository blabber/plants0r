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

#include <stdint.h>
#include <avr/interrupt.h>

#include "usart.h"
#include "buffer.h"

#define BAUD	UA_BAUD
#include <util/setbaud.h>

volatile uint8_t UA_RX_done = 0;

struct buffer *out;
struct buffer *in;

void
UA_init(void)
{
	UA_ENABLE_TX();
	UA_ENABLE_RX();
	UA_ENABLE_RX_COMPLETE_IRQ();
	UA_SET_TRANSMIT_MODE_8N1();
	UA_SET_BAUD_RATE();

	out = buf_create(UA_BUFFLEN);
	in = buf_create(UA_BUFFLEN);
}

void
UA_putc(uint8_t c)
{
	while (buf_putc(out, c) != 0)
		/* nop */;

	UA_ENABLE_TX_BUFF_READY_IRQ();
}

void
UA_puts(char *s)
{
	char *l = s;
	while (*l != '\0')
		UA_putc(*(l++));
}

uint8_t
UA_gets(char *buffer, uint8_t bufflen)
{
	uint8_t c, l = 0;
	char *b = buffer;

	while ((b < buffer + bufflen) && (buf_getc(in, &c) == 0)) {
		if (c == '\r') {
			*(b++) = '\0';
			break;
		}

		l++;
		*(b++) = c;
	}

	if (UA_RX_done > 0)
		UA_RX_done--;

	return (l);
}

ISR(UA_TX_BUFF_READY_VECTOR)
{
	uint8_t c;
	if (buf_getc(out, &c) == 0)
		UA_TX_BUFF = c;
	else
		UA_DISABLE_TX_BUFF_READY_IRQ();
}

ISR(UA_RX_COMPLETE_VECTOR)
{
	uint8_t c = UA_TX_BUFF;

	if (buf_putc(in, c) == -1) {
		UA_RX_done++;
		return;
	}

	if (c == '\r')
		UA_RX_done++;
}
