/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#include <util/delay.h>

#include <stdbool.h>
#include <stdint.h>

#include "dht11.h"

static void begin_communication(void);
static void read_byte(uint8_t *b);
static bool cksum_is_valid(struct DHT_data *data, uint8_t cksum);
static void stop_communication(void);

void
DHT_init(void)
{
	DHT_DDR |= DHT;
	DHT_PORT |= DHT;
	_delay_ms(2000);
}

int8_t
DHT_read(struct DHT_data *data)
{
	begin_communication();

	read_byte(&(data->humidity_integral));
	read_byte(&(data->humidity_decimal));
	read_byte(&(data->temperature_integral));
	read_byte(&(data->temperature_decimal));

	uint8_t cksum;
	read_byte(&cksum);

	stop_communication();

	if (!cksum_is_valid(data, cksum))
		return (-1);

	return (0);
}

static void
begin_communication(void)
{
	/*
	 * -.       .-------.       .-------.
	 *   \_____/         \_____/         \_
	 *
	 *  |-- 1 --|-- 2 --|-- 3 --|-- 4 --|...
	 *
	 * 1: MCU - send start signal, pull down for at l8ms
	 * 2: MCU - pull up, wait for DHT response (20-40us)
	 * 3: DHT - send out response signal, low for 80us
	 * 4: DHT - pulls up for 80us
	 */

	/* 1: MCU - send start signal, pull down for at l8ms */
	DHT_DDR |= DHT;
	DHT_PORT &= ~(DHT);
	_delay_ms(20);

	/* 2: MCU - pull up, wait for DHT response (20-40us) */
	DHT_DDR &= ~(DHT);	/* pull-up-resistor pulls line up */
	_delay_us(5);		/* give line some time to reach high state */
	while (DHT_PIN & DHT);

	/* 3: DHT - send out response signal, low for 80us */
	while (!(DHT_PIN & DHT));

	/* 4: DHT - pulls up for 80us */
	while (DHT_PIN & DHT);
}

static void
read_byte(uint8_t *b)
{
	/*
	 * -.       .-------.       .-------.
	 *   \_____/         \_____/         \_
	 *
	 *  |-- 1 --|-- 2 --|-- 1 --|-- 2 --|...
	 *
	 * 1: DHT - start to transmit 1-bit data (50us)
	 * 2: DHT - pull up, 26-28us means 0, 70us means 1
	 *
	 * (repeat for the other bits)
	 */

	for (uint8_t i = 0; i < 8; i++) {
		/* shift to next bit position */
		*b <<= 1;

		 /* 1: DHT - start to transmit 1-bit data (50us) */
		while (!(DHT_PIN & DHT));

		 /* 2: DHT - pull up, 26-28us means 0, 70us means 1 */
		_delay_us(30);
		if (DHT_PIN & DHT)
			*b |= 0x01;
		while (DHT_PIN & DHT);
	}
}

static bool
cksum_is_valid(struct DHT_data *data, uint8_t cksum)
{
	/*
	 * If the data transmission was successful, the checksum will be the last 8
	 * bit of the sum of all four datapoints read.
	 */

	uint16_t sum = data->humidity_integral + data->humidity_decimal +
	    data->temperature_integral + data->temperature_decimal;

	if (cksum != (sum & 0xff))
		return (false);

	return (true);
}

static void stop_communication(void)
{
	DHT_init();
}
