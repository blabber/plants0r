/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "usart.h"
#include "dht11.h"

#define BUFFLEN 32

#define LED (1<<PB5)

int
main(void)
{
	DDRB |= LED;

	sei();

	DHT_init();
	UA_init();

	while (DHT_get_state() != DHT_IDLE)
		/* nop */;

	struct DHT_data dht;

	DHT_read(&dht);
	for (;;) {
		if (DHT_get_state() == DHT_IDLE) {
			if (dht.valid_reading)
				PORTB &= ~(LED);
			else {
				PORTB |= LED;
				UA_puts("failed reading: ");
			}

			char buffer[BUFFLEN];
			snprintf(buffer, BUFFLEN, "%d.%d%% %d.%ddegC\r\n",
			    dht.humidity_integral, dht.humidity_decimal,
			    dht.temperature_integral, dht.temperature_decimal);

			UA_puts(buffer);

			DHT_read(&dht);
		}
	}

	/* NOTREACHED */
	return (0);
}
