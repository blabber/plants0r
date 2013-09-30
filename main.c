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
#include "adc.h"

#define BUFFLEN 48

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
		else {
			PORTB |= LED;
			if (dht.timeout)
				UA_puts("timeout: ");
			else
				UA_puts("failed reading: ");
		}

		uint16_t ldr = ADC_read(LDR);
		uint16_t moisture = ADC_read(MST);

		snprintf(buffer, BUFFLEN, "%d.%d%% %d.%ddegC ldr: %d moisture: "
		    "%d\r\n", dht.humidity_integral, dht.humidity_decimal,
		    dht.temperature_integral, dht.temperature_decimal, ldr,
		    moisture);

		UA_puts(buffer);
	}

	/* NOTREACHED */
	return (0);
}
