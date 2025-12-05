#ifndef POWER_H
#define POWER_H

#include "peripheral.h"

enum{
    PM_BASE = (PERIPHERAL_BASE + PM_OFFSET),

    PM_RSTC = (PM_BASE + 0x1C),
    PM_RSTS = (PM_BASE + 0x20),
    PM_WDOG = (PM_BASE + 0x24)
};

enum{
    PM_WDOG_MAGIC = 0x5a000000,
    PM_RSTC_FULLRST = 0x00000020
};

void power_off();
void reset();

#endif