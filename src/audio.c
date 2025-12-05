#include "gpio.h"
#include "uart.h"
#include "clock.h"
#include "audio.h"
#include "kerio.h"
#include "delay.h"
#include "mmio.h"

void audio_init(void){
    //Setup PWM
    unsigned int r = mmio_read(GPFSEL4);
    r &= ~(7<<0 | 7<<3);
    r |= (4<<0 | 4<<3);
    mmio_write(GPFSEL4, r);
    wait_msec(2);

    r = CLOCK_PASS | (1<<5);
    mmio_write(PWMCLK_CNTL, r);
    wait_msec(2);

    r=CLOCK_PASS | PWM_CLOCK_DIVI<<12;
    mmio_write(PWMCLK_DIV, r);
    mmio_write(PWMCLK_CNTL, (CLOCK_PASS | 1<<4 | 1)); //Set Clock to oscillator and Enable it
    wait_msec(2);
    
    mmio_write(PWM_CTL, 0);
    wait_msec(2);
    
    mmio_write(PWM_RNG1, PWM_RANGE);
    mmio_write(PWM_RNG2, PWM_RANGE);

    r = 1 //Enable Channel 1
        | 1<<5 //Use FIFO for Channel 1
        | 1<<8 //Enable Channel 2
        | 1<<13 //Use FIFO for Channel 1
        | 1<<6; // Clear FIFO
    mmio_write(PWM_CTL, r);
    printf("AUDIO: Initialized\n");
}

void playaudio(unsigned char *data, unsigned int size){
    int i=0;
    long status;

    while(i<size){
        status = mmio_read(PWM_STA);
        if(!(status & PWM_STA_FULL)){
            mmio_write(PWM_FIFO, *(data + i));
            i++;
            mmio_write(PWM_FIFO, *(data + i));
            i++;
        }
        if(status & PWM_STA_ERROR_MASK) mmio_write(PWM_STA, PWM_STA_ERROR_MASK);

    }
}