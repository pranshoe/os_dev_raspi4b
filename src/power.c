#include "mbox.h"
#include "uart.h"
#include "gpio.h"
#include "power.h"
#include "mmio.h"

void power_off(){
    
    
    for(unsigned int i=0; i<16; i++){
        mbox[0]=8*4;
        mbox[1]=MBOX_REQUEST;
        mbox[2]=MBOX_TAG_SETPOWER;
        mbox[3]=8;
        mbox[4]=8;
        mbox[5]=i;
        mbox[6]=0;
        mbox[7]=MBOX_TAG_LAST;

        mbox_call(MBOX_CH_PROP);
    }

    //Power off GPIO pins
    mmio_write(GPFSEL0, 0);
    mmio_write(GPFSEL1, 0);
    mmio_write(GPFSEL2, 0);
    mmio_write(GPFSEL3, 0);
    mmio_write(GPFSEL4, 0);
    mmio_write(GPFSEL5, 0);

    mmio_write(GPPUPPDN0, 0);
    mmio_write(GPPUPPDN1, 0);
    mmio_write(GPPUPPDN2, 0);
    mmio_write(GPPUPPDN3, 0);


    unsigned int temp = mmio_read(PM_RSTS);
    temp &= ~0xfffffaaa;
    temp |= 0x555; //Partition 63 indicates halt
    
    mmio_write(PM_RSTC, PM_WDOG_MAGIC | temp);
    mmio_write(PM_WDOG, PM_WDOG_MAGIC | 10);
    mmio_write(PM_RSTC, PM_WDOG_MAGIC | PM_RSTC_FULLRST);
    
}

void reset(){
    unsigned int temp;

    temp = mmio_read(PM_RSTS);
    temp &= ~0xfffffaaa;
    //Boot from Partition 0 
    
    mmio_write(PM_RSTC, PM_WDOG_MAGIC | temp);
    mmio_write(PM_WDOG, PM_WDOG_MAGIC | 10);
    mmio_write(PM_RSTC, PM_WDOG_MAGIC | PM_RSTC_FULLRST);
}