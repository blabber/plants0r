/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>
#include <avr/io.h>

#define ADC_SET_REFERENCE( )	(ADMUX |= (1<<REFS0))
#define ADC_RESET_CHANNEL()	(ADMUX &= ~(0x0F))
#define ADC_SET_CHANNEL(C)	(ADMUX |= (C))
#define ADC_ENABLE( )		(ADCSRA |= (1<<ADEN))
#define ADC_START( )		(ADCSRA |= (1<<ADSC))
#define ADC_CONVERSION_RUNNING	(ADCSRA & (1<<ADSC))
#define ADC_SET_PRESCALER()	(ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0))
#define ADC_VALUE		(ADC)

/*
 * If your AVR does not support ADC noise reduction mode, define
 * ADC_POLL_ACTUAL_READING and ignore the following defines.
 */
#define ADC_ENABLE_ADC_IRQ()	(ADCSRA |= (1<<ADIE))
#define ADC_DISABLE_ADC_IRQ()	(ADCSRA &= ~(1<<ADIE))
#define ADC_ISR_ADC_VECTOR	ADC_vect


void ADC_init(void);
uint16_t ADC_read(uint8_t channel);

#endif
