/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "dht11.h"

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
 *
 *
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

enum states {
	UNINITIALIZED,
	INITIALIZE,
	INITIALIZING,
	INITIALIZED,
	IDLE,
	START_SEND,
	START_SENDING,
	START_SENT,
	RESPONSE_WAITING1,
	RESPONSE_RECEIVED1,
	RESPONSE_WAITING2,
	RESPONSE_RECEIVED2,
	RESPONSE_WAITING3,
	RESPONSE_RECEIVED3,
	READ_WAITING,
	READ_RECEIVING,
	READ_RECEIVED,
	RECOVER,
	RECOVERING,
	RECOVERED,
};

static void begin_communication(void);
static void read_byte(uint8_t *b);
static bool cksum_is_valid(struct DHT_data *data, uint8_t cksum);
static void stop_communication(void);
static enum states handle_state_updates(void);

volatile enum states state = UNINITIALIZED;

void
DHT_init(void)
{
	if (state != UNINITIALIZED)
		return;
	state = INITIALIZE;

	while ( handle_state_updates() != IDLE);
}

static enum states
handle_state_updates(void)
{
	switch(state) {
	case UNINITIALIZED:
		break;

	case INITIALIZE:
	case RECOVER:
		/* pull up data line for at least 1 sec */
		DHT_DDR |= DHT;
		DHT_PORT |= DHT;

		DHT_TCNT = 0;
		DHT_OCR = DHT_TICKS_PER_SEC;
		DHT_ENABLE_OCR_IRQ();
		DHT_START_TIMER_SEC();

		if (state == INITIALIZE)
			state = INITIALIZING;
		else
			state = RECOVERING;
		break;

	case INITIALIZING:
	case RECOVERING:
		break;

	case INITIALIZED:
	case RECOVERED:
		DHT_DISABLE_OCR_IRQ();
		DHT_STOP_TIMER_SEC();

		state = IDLE;
		break;

	case START_SEND:
		/* 1: MCU - send start signal, pull down for at l8ms */
		DHT_DDR |= DHT;
		DHT_PORT &= ~(DHT);

		DHT_TCNT = 0;
		DHT_OCR = DHT_TICKS_PER_MS * 20;
		DHT_ENABLE_OCR_IRQ();
		DHT_START_TIMER_MS();

		state = START_SENDING;
		break;

	case START_SENDING:
		break;

	case START_SENT:
		DHT_DISABLE_OCR_IRQ();
		DHT_STOP_TIMER_MS();

		/* 2: MCU - pull up, wait for DHT response (20-40us) */
		DHT_DDR &= ~(DHT); /* external pull-up-resistor keeps line up */
		DHT_ENABLE_INT_FALL_IRQ();

		state = RESPONSE_WAITING1;
		break;

	case RESPONSE_WAITING1:
		break;

	case RESPONSE_RECEIVED1:
		DHT_DISABLE_INT_FALL_IRQ();

		/* 3: DHT - send out response signal, low for 80us */
		DHT_ENABLE_INT_RISE_IRQ();

		state = RESPONSE_WAITING2;
		break;

	case RESPONSE_WAITING2:
		break;

	case RESPONSE_RECEIVED2:
		DHT_DISABLE_INT_RISE_IRQ();

		/* 4: DHT - pulls up for 80us */
		DHT_ENABLE_INT_FALL_IRQ();

		state = RESPONSE_WAITING3;
		break;

	case RESPONSE_WAITING3:
		break;

	ase RESPONSE_RECEIVED3:
		DHT_DISABLE_INT_FALL_IRQ();

		state = READ_WAITING;
		break;

	default:
		/* TODO muss weg */
		break;
	}

	return (state);
}



int8_t
DHT_read(struct DHT_data *data)
{
	if (state != IDLE)
		return (-1);

	state = START_SEND;

	while (handle_state_updates() != READ_WAITING);
		/* nop */

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
read_byte(uint8_t *b)
{

	for (uint8_t i = 0; i < 8; i++) {
		/* shift to next bit position */
		*b <<= 1;

		 /* 1: DHT - start to transmit 1-bit data (50us) */
		state = READ_WAITING;

		DHT_ENABLE_INT_RISE_IRQ();

		while (state != READ_RECEIVING)
			/* nop */;

		DHT_DISABLE_INT_RISE_IRQ();
		DHT_ENABLE_INT_FALL_IRQ();
		DHT_TCNT = 0;
		DHT_START_TIMER_US();

		while (state != READ_RECEIVED)
			/* nop */;

		uint16_t time = DHT_TCNT;
		DHT_STOP_TIMER_US();
		DHT_DISABLE_INT_FALL_IRQ();

		 /* 2: DHT - pull up, 26-28us means 0, 70us means 1 */
		if (time >= DHT_TICKS_PER_US * 50)
			*b |= (0x01);
		else
			*b &= ~(0x01);
	}
}

static bool
cksum_is_valid(struct DHT_data *data, uint8_t cksum)
{
	/*
	 * If the data transmission was successful, the checksum will be the last 8
	 * bit of the sum of all four datapoints read.
	 */

	if (((data->humidity_integral + data->humidity_decimal +
	    data->temperature_integral + data->temperature_decimal) & 0xff)
	    != cksum)
		return (false);

	return (true);
}

static void stop_communication(void)
{
	state = RECOVERING;

	/* pull up data line for at least 1 sec */
	DHT_DDR |= DHT;
	DHT_PORT |= DHT;

	DHT_TCNT = 0;
	DHT_OCR = DHT_TICKS_PER_SEC;
	DHT_ENABLE_OCR_IRQ();
	DHT_START_TIMER_SEC();

	while (handle_state_updates() != IDLE)
		/* nop */;
}

ISR(DHT_ISR_OCR_VECTOR)
{
	switch (state) {
	case INITIALIZING:
		state = INITIALIZED;
		break;
	case RECOVERING:
		state = RECOVERED;
		break;
	case START_SENDING:
		state = START_SENT;
		break;
	default:
		break;
	}
}

ISR(DHT_ISR_INT_VECTOR)
{
	switch (state) {
	case RESPONSE_WAITING1:
		state = RESPONSE_RECEIVED1;
		break;
	case RESPONSE_WAITING2:
		state = RESPONSE_RECEIVED2;
		break;
	case RESPONSE_WAITING3:
		state = RESPONSE_RECEIVED3;
		break;
	case READ_WAITING:
		state = READ_RECEIVING;
		break;
	case READ_RECEIVING:
		state = READ_RECEIVED;
		break;
	default:
		break;
	}
}
