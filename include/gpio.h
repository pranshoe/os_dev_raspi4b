#ifndef GPIO_H
#define GPIO_H

#include "peripheral.h"

// Good Doc: https://elinux.org/RPi_BCM2711_GPIOs
enum{
    GPIO_BASE = (PERIPHERAL_BASE + GPIO_OFFSET),

    //Defining GPIO PINS
    GPFSEL0 = (GPIO_BASE),
    GPFSEL1 = (GPIO_BASE + 0x04),
    GPFSEL2 = (GPIO_BASE + 0x08),
    GPFSEL3 = (GPIO_BASE + 0x0C),
    GPFSEL4 = (GPIO_BASE + 0x10),
    GPFSEL5 = (GPIO_BASE + 0x14),
    GPSET0 = (GPIO_BASE + 0x1C),
    GPSET1 = (GPIO_BASE + 0x20),
    GPCLR0 = (GPIO_BASE + 0x28),
    GPLEV0 = (GPIO_BASE + 0x34),
    GPLEV1 = (GPIO_BASE + 0x38),
    GPEDS0 = (GPIO_BASE + 0x40),
    GPEDS1 = (GPIO_BASE + 0x44),
    GPHEN0 = (GPIO_BASE + 0x64),
    GPHEN1 = (GPIO_BASE + 0x68),
    GPPINMUXSD = (GPIO_BASE + 0xD0),
    GPPUPPDN0 = (GPIO_BASE + 0xE4),
    GPPUPPDN1 = (GPIO_BASE + 0xE8),
    GPPUPPDN2 = (GPIO_BASE + 0xEC),
    GPPUPPDN3 = (GPIO_BASE + 0xF0)
};

#endif