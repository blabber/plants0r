/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#include <stdint.h>
#include <stdlib.h>

#include "buffer.h"


struct buffer {
	uint8_t *data;
	uint8_t size;
	volatile uint8_t writepos;
	volatile uint8_t readpos;
};

struct buffer *
buf_create(uint8_t size)
{
	struct buffer *b = malloc(sizeof(struct buffer));
	if (b == NULL)
		return (NULL);

	b->data = malloc(size);
	if (b->data == NULL) {
		free(b);
		return (NULL);
	}

	b->size = size;
	b->writepos = b->readpos = 0;

	return (b);
}

void
buf_destroy(struct buffer *b)
{
	free(b->data);
	free(b);
}

int8_t
buf_putc(struct buffer *b, uint8_t c)
{
	uint8_t twp = (b->writepos + 1) % b->size;
	if (twp == b->readpos)
		return (-1);

	b->data[b->writepos] = c;
	b->writepos = twp;

	return (0);
}

int8_t
buf_getc(struct buffer *b, uint8_t *c)
{
	if (b->readpos == b->writepos)
		return (-1);

	*c = b->data[b->readpos];
	b->readpos = (b->readpos + 1) % b->size;

	return (0);
}

int8_t
buf_is_full(struct buffer *b)
{
	uint8_t twp = (b->writepos + 1) % b->size;
	if (twp == b->readpos)
		return (1);

	return (0);
}
