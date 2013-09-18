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

#define DHT_DDR		DDRD
#define DHT_PORT	PORTD
#define DHT_PIN		PIND
#define DHT		(1<<PD2)

#define DHT_PRESCALE_SEC	(1024)
#define DHT_TICKS_PER_SEC	(F_CPU / DHT_PRESCALE_SEC)
#define DHT_START_TIMER_SEC( )	(TCCR1B |= (1<<CS12) | (1<<CS10))
#define DHT_STOP_TIMER_SEC( )	(TCCR1B &= ~((1<<CS12) | (1<<CS10)))

#define DHT_PRESCALE_MS		(64)
//#define DHT_TICKS_PER_MS	(F_CPU / (DHT_PRESCALE_MS * 1000))
#define DHT_TICKS_PER_MS	((F_CPU / DHT_PRESCALE_MS) / 1000)
#define DHT_START_TIMER_MS( )	(TCCR1B |= (1<<CS11) | (1<<CS10))
#define DHT_STOP_TIMER_MS( )	(TCCR1B &= ~((1<<CS11) | (1<<CS10)))

#define DHT_PRESCALE_US		(1)
#define DHT_TICKS_PER_US	(F_CPU / (DHT_PRESCALE_US * 1000000))
#define DHT_START_TIMER_US( )	(TCCR1B |= (1<<CS10))
#define DHT_STOP_TIMER_US( )	(TCCR1B &= ~(1<<CS10))

#define DHT_TCNT		TCNT1
#define DHT_OCR			OCR1A
#define DHT_ENABLE_OCR_IRQ( )	(TIMSK1 |= (1<<OCIE1A))
#define DHT_DISABLE_OCR_IRQ( )	(TIMSK1 &= ~(1<<OCIE1A))
#define DHT_ISR_OCR_VECTOR	TIMER1_COMPA_vect

#define DHT_ISR_INT_VECTOR		INT0_vect
#define DHT_ENABLE_INT_RISE_IRQ( )	do {				\
						EICRA |= ((1<<ISC01) |	\
						    (1<<ISC00));	\
						EIMSK |= (1<<INT0);	\
					} while (0)
#define DHT_DISABLE_INT_RISE_IRQ( )	do {				\
						EICRA &= ~((1<<ISC01) |	\
						    (1<<ISC00));	\
						EIMSK &= ~(1<<INT0);	\
					} while (0)
#define DHT_ENABLE_INT_FALL_IRQ( )	do {				\
						EICRA |= (1<<ISC01);	\
						EIMSK |= (1<<INT0);	\
					} while (0)
#define DHT_DISABLE_INT_FALL_IRQ( )	do {				\
						EICRA &= ~(1<<ISC01);	\
						EIMSK &= ~(1<<INT0);	\
					} while (0)

struct DHT_data {
	uint8_t humidity_integral;
	uint8_t humidity_decimal;
	uint8_t temperature_integral;
	uint8_t temperature_decimal;
};

void DHT_init(void);
int8_t DHT_read(struct DHT_data *data);

#endif
