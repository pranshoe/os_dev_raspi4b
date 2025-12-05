#include "gpio.h"
#include "mmio.h"
#include "uart.h"
#include "mbox.h"
#include <stdint.h>

void uart_init(){
    unsigned int temp;

    mmio_write(UART0_CR, 0x0);

    //Set Clock Rate to 4Mhz
    mbox[0] = 9*4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = MBOX_TAG_SETCLKRATE;
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2;
    mbox[6] = 4000000;
    mbox[7] = 0; //No turbo
    mbox[8] = MBOX_TAG_LAST;
    mbox_call(MBOX_CH_PROP);

    //Enable GPIO14 and GPIO15 pins
    temp = mmio_read(GPFSEL1);
    temp&=~((7<<12)|(7<<15));
    temp|=(4<<12)|(4<<15);
    mmio_write(GPFSEL1, temp);

    //Disable pull up/down for pins 14 and 15
    temp = mmio_read(GPPUPPDN0);
    temp &= ~((3<<28) | (3<<30));
    mmio_write(GPPUPPDN0, temp);

    //Clear Interrupts
    mmio_write(UART0_ICR, 0x7FF);

    //115200 baud
    mmio_write(UART0_IBRD, 2);
    mmio_write(UART0_FBRD, 0xB);

    //Enable FIFO and set Word Length to 8 bits(8n1 config)
    mmio_write(UART0_LCRH, 0x7<<4);

    //Enable Transmit/Receive and UART0
    mmio_write(UART0_CR, 0x301);

}


void uart_putc(unsigned int c){
    while(mmio_read(UART0_FR) & 0x20);

    mmio_write(UART0_DR, c);
}

char uart_getc(){
    while(mmio_read(UART0_FR) & 0x10);

    return (char)mmio_read(UART0_DR);
}

void uart_puts(char *s){
    while(*s){
        if(*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}
