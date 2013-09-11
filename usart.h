/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#ifndef _USART_H_
#define _USART_H_

#include <avr/io.h>

void UA_init(void);
void UA_putc(uint8_t c);
void UA_puts(char *s);

#endif
