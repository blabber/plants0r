#ifndef _USART_H_
#define _USART_H_

#include <avr/io.h>

void UA_init(void);
void UA_putc(char *c);
void UA_puts(char *s);

#endif
