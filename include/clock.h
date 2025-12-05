#ifndef CLOCK_H
#define CLOCK_H

#include "peripheral.h"

#define CLOCK_PASS 0x5A000000

enum{
    PWMCLK_BASE = (PERIPHERAL_BASE + CLOCK_OFFSET),

    PWMCLK_CNTL = (PWMCLK_BASE + 0xA0),
    PWMCLK_DIV = (PWMCLK_BASE + 0xA4)
};

#endif
