/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "dht11.h"

/*
 *   .----------------------------------
 * _/
 *
 *  |-- 1 -------------|-- 2 -----------
 *
 * 1: MCU - Pull up data for at least one second
 *    (INITIALZE -> INITIALIZING -> INITIALIZED)
 * 2: MCU - Idle
 *    (IDLE)
 *
 *
 * -.       .-------.       .-------.
 *   \_____/         \_____/         \_
 *
 *  |-- 3 --|-- 4 --|-- 5 --|-- 6 --|...
 *
 * 3: MCU - send start signal, pull down for at least 18ms
 *    (START_SEND -> START_SENDING -> START_SENT)
 * 4: MCU - pull up, wait for DHT response (20-40us)
 *    (START_SENT -> RESPONSE_WAITING1 -> RESPONSE_RECEIVED1)
 * 5: DHT - send out response signal, low for 80us
 *    (RESPONSE_RECEIVED1 -> RESPONSE_WAITING2 -> RESPONSE_RECEIVED2)
 * 6: DHT - pulls up for 80us
 *    (RESPONSE_RECEIVED2 -> RESPONSE_WAITING3 -> RESPONSE_RECEIVED3)
 *
 *
 * -.       .-------.       .-------.
 *   \_____/         \_____/         \_
 *
 *  |-- 7 --|-- 8 --|-- 7 --|-- 8 --|...
 *
 * 7: DHT - start to transmit 1-bit data (50us)
 *    (READ_WAIT -> READ_WAITING)
 * 8: DHT - pull up, 26-28us means 0, 70us means 1
 *    (READ_RECEIVE -> READ_RECEIVING -> READ_RECEIVED)
 *
 * (repeat for the other bits)
 *
 *
 * X: Not part of communication protocol - calculate checksum
 *    (CHECKSUM)
 *
 *
 *   .----------------------------------
 * _/
 *
 *  |-- 1 -------------|-- 2 -----------
 *
 * 1: MCU - Keep up data for at least one second
 *    (RECOVER -> RECOVERING -> RECOVERED)
 * 2: MCU - Idle
 *    (IDLE)
 *
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
	READ_WAIT,
	READ_WAITING,
	READ_RECEIVE,
	READ_RECEIVING,
	READ_RECEIVED,
	CHECKSUM,
	RECOVER,
	RECOVERING,
	RECOVERED,
};

static enum states handle_state_updates(void);

volatile static enum states state = UNINITIALIZED;

static struct DHT_data *dht_data;
static uint8_t read_bits = 0;
static uint8_t cksum = 0;

void
DHT_init(void)
{
	if (state != UNINITIALIZED)
		return;
	state = INITIALIZE;
}

static enum states
handle_state_updates(void)
{
	uint8_t *b = NULL;

	switch(state) {
	case UNINITIALIZED:
		break;

	case INITIALIZE:
	case RECOVER:
		/* 1: MCU - Pull up data for at least one second */
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

	case IDLE:
		/* 2: MCU - Idle */
		break;

	case START_SEND:
		/* 3: MCU - send start signal, pull down for at least 18ms */
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

		/* 4: MCU - pull up, wait for DHT response (20-40us) */
		DHT_DDR &= ~(DHT); /* external pull-up-resistor keeps line up */
		DHT_ENABLE_INT_FALL_IRQ();

		state = RESPONSE_WAITING1;
		break;

	case RESPONSE_WAITING1:
		break;

	case RESPONSE_RECEIVED1:
		DHT_DISABLE_INT_FALL_IRQ();

		/* 5: DHT - send out response signal, low for 80us */
		DHT_ENABLE_INT_RISE_IRQ();

		state = RESPONSE_WAITING2;
		break;

	case RESPONSE_WAITING2:
		break;

	case RESPONSE_RECEIVED2:
		DHT_DISABLE_INT_RISE_IRQ();

		/* 6: DHT - pulls up for 80us */
		DHT_ENABLE_INT_FALL_IRQ();

		state = RESPONSE_WAITING3;
		break;

	case RESPONSE_WAITING3:
		break;

	case RESPONSE_RECEIVED3:
		DHT_DISABLE_INT_FALL_IRQ();

		state = READ_WAIT;
		break;

	case READ_WAIT:
		/* 7: DHT - start to transmit 1-bit data (50us) */
		DHT_ENABLE_INT_RISE_IRQ();

		state = READ_WAITING;
		break;

	case READ_WAITING:
		break;

	case READ_RECEIVE:
		DHT_DISABLE_INT_RISE_IRQ();

		/* 8: DHT - pull up, 26-28us means 0, 70us means 1 */
		DHT_ENABLE_INT_FALL_IRQ();
		DHT_TCNT = 0;
		DHT_START_TIMER_US();

		state = READ_RECEIVING;
		break;

	case READ_RECEIVING:
		break;

	case READ_RECEIVED:
		DHT_STOP_TIMER_US();
		DHT_DISABLE_INT_FALL_IRQ();

		if (read_bits < 8)
			b = &(dht_data->humidity_integral);
		else if (read_bits < 16)
			b = &(dht_data->humidity_decimal);
		else if (read_bits < 24)
			b = &(dht_data->temperature_integral);
		else if (read_bits < 32)
			b = &(dht_data->temperature_decimal);
		else if (read_bits < 40)
			b = &cksum;

		*b <<= 1;
		/* 26-28us means 0, 70us means 1 */
		if (DHT_TCNT >= DHT_TICKS_PER_US * 50)
			*b |= (0x01);

		if (++read_bits < 40)
			state = READ_WAIT;
		else
			state = CHECKSUM;
		break;
	case CHECKSUM:
		/*
		 * If the data transmission was successful, the checksum will be
		 * the last 8 bit of the sum of all four datapoints read.
		 */

		if (((dht_data->humidity_integral + dht_data->humidity_decimal +
		    dht_data->temperature_integral +
		    dht_data->temperature_decimal) & 0xff) == cksum)
			dht_data->valid_reading = 1;

		state = RECOVER;
		break;
	}

	return (state);
}



void
DHT_read(struct DHT_data *data)
{
	if (state != IDLE)
		return;

	data->humidity_integral = 0;
	data->humidity_decimal = 0;
	data->temperature_integral = 0;
	data->temperature_decimal = 0;
	data->valid_reading = 0;
	cksum = 0;
	read_bits = 0;

	dht_data = data;
	state = START_SEND;

	handle_state_updates();
}

enum DHT_states
DHT_get_state()
{
	enum states s;

	switch (handle_state_updates()) {
	case UNINITIALIZED:
		s = DHT_UNINITIALIZED;
		break;
	case IDLE:
		s = DHT_IDLE;
		break;
	default:
		s = DHT_BUSY;
		break;
	}

	return (s);
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
		state = READ_RECEIVE;
		break;
	case READ_RECEIVING:
		state = READ_RECEIVED;
		break;
	default:
		break;
	}
}
