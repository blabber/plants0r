/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "adc.h"

#ifndef ADC_POLL_ACTUAL_READING
volatile uint8_t adc_finished = 0;
#endif

void
ADC_init(void)
{
	ADC_SET_REFERENCE();
	ADC_SET_PRESCALER();
	ADC_ENABLE();
}

uint16_t
ADC_read(uint8_t channel)
{
	ADC_RESET_CHANNEL();
	ADC_SET_CHANNEL(channel);
	ADC_START();

	/* dummy reading */
	while (ADC_CONVERSION_RUNNING)
		/* nop */;

	/*
	 * This is the actual reading; serious business using ADC noise
	 * canceler. On some AVRs this might be not availabe, in this case
	 * define ADC_POLL_ACTUAL_READING there.
	 */
#ifndef ADC_POLL_ACTUAL_READING
	adc_finished = 0;
	ADC_ENABLE_ADC_IRQ();
	set_sleep_mode(SLEEP_MODE_ADC);
	sleep_mode();
	while (!adc_finished)
		/* nop */;
	ADC_DISABLE_ADC_IRQ();
#else
	while (ADC_CONVERSION_RUNNING)
		/* nop */;
#endif

	return (ADC_VALUE);
}

#ifndef ADC_POLL_ACTUAL_READING
ISR(ADC_ISR_ADC_VECTOR) {
	/*
	 * Ideally the interrupt itself exits sleep mode and no state variable
	 * would be necessary, but other interrupts might exit sleep mode too.
	 * In this case, fall back to state variable.
	 */
	adc_finished = 1;
}
#endif
