/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#ifndef _DHT11_H_
#define _DHT11_H_

#include <stdint.h>
#include <avr/io.h>

#define DHT_DDR		DDRB
#define DHT_PORT	PORTB
#define DHT_PIN		PINB
#define DHT		(1<<PB0)

struct DHT_data {
	uint8_t humidity_integral;
	uint8_t humidity_decimal;
	uint8_t temperature_integral;
	uint8_t temperature_decimal;
};

void DHT_init(void);
int8_t DHT_read(struct DHT_data *data);

#endif
