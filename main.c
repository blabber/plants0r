/*-
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tobias.rehbein@web.de> wrote this file. As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *                                                             Tobias Rehbein
 */

#include "usart.h"

int
main(void)
{
	UA_init();	

	for (;;) {
		UA_puts("Alive and kicking!\r\n");
	}

	/* NOTREACHED */
	return (0);
}
