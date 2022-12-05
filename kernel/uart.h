#ifndef UART_H__
#define UART_H__

#include <stdint.h>

#define BAUDRATE_115200 115200

void uart_send(const char *str_p);
void uart_init(uint32_t baudrate);
int uart_putchar(char c);
int uart_getchar(void);

#endif
