#include "uart.h"
#include "stdlib.h"
#include "framebuffer.h"
#include <stdarg.h>

void putc(char c){
    // framebuffer_putc(c);
    uart_putc(c);
}

char getc(){
    return uart_getc();
}

void puts(char * str){
    while(*str){
        if(*str == '\n')
            putc('\r');
        putc(*str++);
    }
}

void printf(const char * str, ...){
    va_list args;
    va_start(args, str);

    for(; *str != '\0'; str++){
        if(*(str) == '%'){
            switch(*(++str)){
                case '%':
                    putc('%');
                    break;
                case 'd':
                    puts(itoa(va_arg(args, int), 10));
                    break;
                case 'x':
                    puts(utoa(va_arg(args, int), 16));
                    break;
                case 'o':
                    puts(itoa(va_arg(args, int), 8));
                    break;
                case 's':
                    puts(va_arg(args, char *));
                    break;
                case 'c':
                    putc(va_arg(args, int));
                    break;
            }
        } else {
            if(*(str)=='\n') putc('\r');
            putc(*str);
        }
    }

}