#include "peripheral.h"
#include <stdint.h>

#ifndef UART_H
#define UART_H

/* Auxilary mini UART registers */

enum{
    UART0_BASE = (PERIPHERAL_BASE + UART0_OFFSET),
    
    UART0_DR = (UART0_BASE),
    UART0_FR = (UART0_BASE + 0x18),
    UART0_IBRD = (UART0_BASE + 0x24),
    UART0_FBRD = (UART0_BASE + 0x28),
    UART0_LCRH = (UART0_BASE + 0x2C),
    UART0_CR = (UART0_BASE + 0x30),
    UART0_IMSC = (UART0_BASE + 0x38),
    UART0_ICR = (UART0_BASE + 0x44)
};

void uart_init();
void uart_putc(unsigned int c);
char uart_getc();
void uart_puts(char *s);

#endif