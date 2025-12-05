#ifndef AUDIO_H
#define AUDIO_H

#include "peripheral.h"

#define PWM_CLOCK_DIVI 2
#define PWM_RANGE 0x264

enum{
    PWM_BASE = (PERIPHERAL_BASE + PWM_OFFSET),

    PWM_CTL = (PWM_BASE),
    PWM_STA = (PWM_BASE + 0x04),
    PWM_DMAC = (PWM_BASE + 0x08),
    PWM_RNG1 = (PWM_BASE + 0x10),
    PWM_DAT1 = (PWM_BASE + 0x14),
    PWM_FIFO = (PWM_BASE + 0x18),
    PWM_RNG2 = (PWM_BASE + 0x20),
    PWM_DAT2 = (PWM_BASE + 0x24)
};

enum{
    PWM_STA_FULL = 0x1,
    PWM_STA_GAPO1 = 0x10,
    PWM_STA_GAPO2 = 0x20,
    PWM_STA_RERR1 = 0x8,
    PWM_STA_WERR1 = 0x4,
    PWM_STA_ERROR_MASK = (PWM_STA_GAPO1 | PWM_STA_GAPO2 | PWM_STA_RERR1 | PWM_STA_WERR1)
};

void audio_init(void);
void playaudio(unsigned char * data, unsigned int size);

#endif