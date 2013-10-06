/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "usart.h"
#include "dht11.h"
#include "adc.h"

#define BUFFLEN 16

#define LED (1<<PB5)
#define LDR (0x00)
#define MST (0x01)

int
main(void)
{
	DDRB |= LED;

	sei();

	DHT_init();
	UA_init();
	ADC_init();

	struct DHT_data dht;
	char buffer[BUFFLEN];

	for (;;) {
		DHT_read(&dht);
		if (dht.valid_reading)
			PORTB &= ~(LED);
		else
			PORTB |= LED;

		uint16_t ldr = ADC_read(LDR);
		uint16_t moisture = ADC_read(MST);

		UA_puts("BEGIN\r\n");

		snprintf(buffer, BUFFLEN, "L:%d\r\n", ldr);
		UA_puts(buffer);

		snprintf(buffer, BUFFLEN, "M:%d\r\n", moisture);
		UA_puts(buffer);

		if (dht.valid_reading) {
			snprintf(buffer, BUFFLEN, "T:%d.%d\r\n",
			    dht.temperature_integral, dht.temperature_decimal);
			UA_puts(buffer);

			snprintf(buffer, BUFFLEN, "H:%d.%d\r\n",
			    dht.humidity_integral, dht.humidity_decimal);
			UA_puts(buffer);
		} else {
			if (dht.timeout)
				UA_puts("D:timeout\r\n");
			else
				UA_puts("D:failed\r\n");
		}

		UA_puts("END\r\n");

		for (;;) {
			while (UA_RX_done == 0)
				/* nop */;

			UA_gets(buffer, BUFFLEN);
			if (strncmp(buffer, "read", strlen("read")) == 0)
				break;
		}
	}

	/* NOTREACHED */
	return (0);
}
