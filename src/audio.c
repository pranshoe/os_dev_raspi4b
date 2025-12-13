#include "gpio.h"
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

unsigned int parse_pcm(unsigned char* data, unsigned short byte_depth){
    int raw = 0;
    
    for(int i=0; i<byte_depth; i++){
        raw |= data[i]<<(8*i);
    }
    
    //Convert to 8-bit PCM Audio since our raspi doesn't support any standard bit-depths above 8-bit
    if(byte_depth!=1){
        if(raw & 0x00800000) raw|=0xff000000;
        raw = raw>>16;
        raw = raw +128;
    }
    return raw;
}

void playaudio(unsigned char *data, unsigned int size, unsigned short byte_depth){
    int i=0;
    long status;

    while(i<size){
        status = mmio_read(PWM_STA);
        if(!(status & PWM_STA_FULL)){
            mmio_write(PWM_FIFO, parse_pcm(data + i, byte_depth));
            i+=byte_depth;
            mmio_write(PWM_FIFO, parse_pcm(data + i, byte_depth));
            i+=byte_depth;
        }
        if(status & PWM_STA_ERROR_MASK) mmio_write(PWM_STA, PWM_STA_ERROR_MASK);

    }
}