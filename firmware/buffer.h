/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdint.h>

struct buffer;

struct buffer *buf_create(uint8_t size);
int8_t buf_putc(struct buffer *b, uint8_t c);
int8_t buf_getc(struct buffer *b, uint8_t *c);
int8_t buf_is_full(struct buffer *b);

#endif
