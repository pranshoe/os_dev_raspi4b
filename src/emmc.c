#include "emmc.h"
#include "delay.h"
#include "uart.h"
#include "kerio.h"
#include "gpio.h"
#include "mbox.h"
#include "mmio.h"

unsigned long sd_scr[2], sd_err, sd_rca, sd_hv;

//Waits for the bits in the mask to clear
int sd_status(unsigned int mask){
    int cnt = 500000;
    while((mmio_read(EMMC_STATUS) & mask) && !(mmio_read(EMMC_INTERRUPT) & INT_ERROR_MASK) && cnt--) wait_msec(1);
    return (cnt<=0 || (mmio_read(EMMC_INTERRUPT) & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

int sd_int(unsigned int mask){
    unsigned int r, m=mask | INT_ERROR_MASK;
    int cnt = 1000000;

    while(!(mmio_read(EMMC_INTERRUPT) & m) && cnt--) wait_msec(1);
    r=mmio_read(EMMC_INTERRUPT);

    if(cnt<=0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT)){

        #ifdef DEBUG
        printf("EMMC: Interrupt Register: %x", r);
        #endif

        mmio_write(EMMC_INTERRUPT, r);
        return SD_TIMEOUT;
    }else if(r & INT_ERROR_MASK){
        mmio_write(EMMC_INTERRUPT, r);
        return SD_ERROR;
    }

    mmio_write(EMMC_INTERRUPT, mask);
    return 0;
}

int sd_cmd(unsigned int code, unsigned int arg){
    int r=0;
    sd_err=SD_OK;


    
    if(code & CMD_NEED_APP){
        r=sd_cmd(CMD_APP_CMD | (sd_rca ? CMD_RSPNS_48 : 0), sd_rca);
        if(sd_rca && !r){
            #ifdef DEBUG
            printf("EMMC: ERROR: Unable to send SD APP Command\n");
            #endif

            sd_err = SD_ERROR;
            return 0;
        }
        code &= ~CMD_NEED_APP;
    }

    if(sd_status(SR_CMD_INHIBIT)){
        #ifdef DEBUG
        printf("EMMC: ERROR: EMMC Busy\n");
        #endif
        sd_err=SD_TIMEOUT;
        return 0;
    }

    #ifdef DEBUG
    printf("EMMC: Sending command: %x arg: %x\n", code, arg);
    #endif

    /* Clear any pending interrupts by writing back the current interrupt flags */
    mmio_write(EMMC_INTERRUPT, mmio_read(EMMC_INTERRUPT));
    mmio_write(EMMC_ARG1, arg);
    mmio_write(EMMC_CMDTM, code);

    if(code==CMD_SEND_OP_COND) wait_msec(1000);
    else if(code==CMD_SEND_IF_COND || code==CMD_APP_CMD) wait_msec(100);
    if((r=sd_int(INT_CMD_DONE))){
        #ifdef DEBUG
        printf("EMMC: Error: Failed to send EMMC Command\n");
        #endif
        sd_err=r;
        return 0;
    }

    r=mmio_read(EMMC_RESP0);

    if(code==CMD_GO_IDLE || code==CMD_APP_CMD) return 0;
    else if(code == (CMD_APP_CMD | CMD_RSPNS_48)) return r&SR_APP_CMD;
    else if(code == CMD_SEND_OP_COND) return r;
    else if(code == CMD_SEND_IF_COND) return r==arg?SD_OK : SD_ERROR;
    else if(code == CMD_ALL_SEND_CID) return r | mmio_read(EMMC_RESP1) | mmio_read(EMMC_RESP2) | mmio_read(EMMC_RESP3);
    else if(code == CMD_SEND_REL_ADDR) {
        sd_err = ((r&0x1fff) |
                    ((r&0x2000) << 6) |
                    ((r&0x4000) << 8) |
                    ((r&0x8000) << 8)) & CMD_ERRORS_MASK;
        return  r&CMD_RCA_MASK;
    }
    return r&CMD_ERRORS_MASK;
}

int sd_readblk(unsigned int ba, unsigned char *buffer, unsigned int num){
    int r, c=0, d;
    if(num<1) num=1;

    #ifdef DEBUG
    printf("EMMC: Reading Block address: %x, Count: %x\n", ba, num);
    #endif
    if(sd_status(SR_DAT_INHIBIT)){
        sd_err = SD_TIMEOUT;
        return 0;
    }

    unsigned int *buf = (unsigned int*) buffer;
    if(sd_scr[0] & SCR_SUPP_CCS){
        if(num>1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)){
            sd_cmd(CMD_SET_BLOKCNT, num);
            if(sd_err) return 0;
        }

        mmio_write(EMMC_BLKSIZECNT, (num << 16) | BLOCK_SIZE);
        sd_cmd(num==1 ? CMD_READ_SINGLE : CMD_READ_MULTI, ba);
        if(sd_err) return 0;
    } else{
        mmio_write(EMMC_BLKSIZECNT, (1<<16) | BLOCK_SIZE);
    }

    while(c<num){
        if(!(sd_scr[0] & SCR_SUPP_CCS)){
            sd_cmd(CMD_READ_SINGLE, (ba+c)*512);
            if(sd_err) return 0;
        }

        if((r=sd_int(INT_READ_RDY))){
            #ifdef DEBUG
            printf("EMMC: Error: Timeout waiting for ready to read\n");
            #endif
            sd_err = r;
            return 0;
        }

        for(d=0; d<128; d++)
            buf[d] = mmio_read(EMMC_DATA);
        c++;
        buf+=128;
    }

    //Todo: check for sdsc cards
    if(num>1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (sd_scr[0] & SCR_SUPP_CCS)) sd_cmd(CMD_STOP_TRANS, 0);
    return sd_err!=SD_OK || c!=num ? 0 : num*512;
}


int sd_clk(unsigned int f){
    unsigned int d, c=41666666/f, x, s=32, h=0;
    int cnt = 100000;

    while((mmio_read(EMMC_STATUS) & (SR_CMD_INHIBIT | SR_DAT_INHIBIT)) && cnt--) wait_msec(1);
    if(cnt<=0){
        #ifdef DEBUG
        printf("EMMC: Error: Timeout waiting for inhibit flag\n");
        #endif
        return SD_ERROR;
    }

    mmio_write(EMMC_CONTROL1, 
        mmio_read(EMMC_CONTROL1) & ~C1_CLK_EN
    );

    wait_msec(10);
    x = c-1;
    if(!x) s=0;
    else{
        if(!(x & 0xffff0000u)) {x <<= 16; s-=16;}
        if(!(x & 0xff000000u)) {x <<= 8; s-=8;}
        if(!(x & 0xf0000000u)) {x <<= 4; s-=4;}
        if(!(x & 0xc0000000u)) {x <<= 2; s-=2;}
        if(!(x & 0x80000000u)) {x <<= 1; s-=1;}
        if(s>0) s--;
        if(s>7) s=7;
    }

    if(sd_hv>HOST_SPEC_V2) d=c;
    else d = (1<<s);

    if(d<=2) {
        d=2;
        s=0;
    }

    #ifdef DEBUG
    printf("EMMC: sd_clk: Clock Divisor: %x, Shift: %x\n", d, s);
    #endif

    if(sd_hv>HOST_SPEC_V2) h=(d&0x300)>>2;
    d=((d&0xff) << 8) | h;

    mmio_write(EMMC_CONTROL1, 
        (mmio_read(EMMC_CONTROL1) & 0xffff003f) | d
    );
    wait_msec(10);

    mmio_write(EMMC_CONTROL1,
        mmio_read(EMMC_CONTROL1) | C1_CLK_EN
    );
    wait_msec(10);

    cnt=10000;
    while(!(mmio_read(EMMC_CONTROL1) & C1_CLK_STABLE) && cnt--) wait_msec(10);
    if(cnt<=0){
        printf("EMMC: Error: Failed to get a stable clock rate\n");
        return SD_ERROR;
    }

    return SD_OK;
}

int sd_init(){
    unsigned long r, cnt, ccs=0;

    //MUX SD
    mmio_write(GPPINMUXSD, 3);

    //GPIO_CD
    r = mmio_read(GPFSEL4);
    r &= ~(7<<21);
    mmio_write(GPFSEL4, r);

    r = mmio_read(GPPUPPDN2);
    r &= ~(3 << 30);
    r |= (1 << 30);
    mmio_write(GPPUPPDN2, r);

    r = mmio_read(GPHEN1);
    r |= 1<<15;
    mmio_write(GPHEN1, r);


    //GPIO_CLK, GPIO_CMD
    r = mmio_read(GPFSEL4);
    r |= (7<<24) | (7<<27);
    mmio_write(GPFSEL4, r);

    r = mmio_read(GPPUPPDN3);
    r &= ~((3 << 0) | (3 << 2));
    r |= ((2 << 0) | (1 << 2));
    mmio_write(GPPUPPDN3, r);

    //GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r = mmio_read(GPFSEL5);
    r |= (7<<0) | (7<<3) | (7<<6) | (7<<9);
    mmio_write(GPFSEL5, r);

    r=mmio_read(GPPUPPDN3);
    r &= ~((3<<4) | (3<<6) | (3<<8) | (3<<10));
    r |= ((1<<4) | (1<<6) | (1<<8) | (1<<10));
    mmio_write(GPPUPPDN3, r);

    sd_hv = (mmio_read(EMMC_SLOTISR_VER) & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;
    #ifdef DEBUG
    printf("EMMC: GPIO set up\n");
    #endif
    
    //Reset the card
    //Disable 1.8V Supply
    mbox[0]=32;
    mbox[1]=MBOX_REQUEST;
    mbox[2]=MBOX_TAG_SETGPIOSTATE;
    mbox[3]=8;
    mbox[4]=8;
    mbox[5]=132;
    mbox[6]=0;
    mbox[7]=MBOX_TAG_LAST;
    
    if(mbox_call(MBOX_CH_PROP)){
        #ifdef DEBUG
        printf("EMMC: Disabled 1.8V Supply\n");
        #endif
    }

    mmio_write(EMMC_CONTROL1, 
        mmio_read(EMMC_CONTROL1) | C1_SRST_HC
    );

    cnt = 10000;
    do{
        wait_msec(10);
    } while((mmio_read(EMMC_CONTROL1) & C1_SRST_HC) && cnt--);
    if(cnt<=0){
        #ifdef DEBUG
        printf("EMMC: Error: Failed to reset EMMC\n");
        #endif
        return SD_ERROR;
    }
    #ifdef DEBUG
    printf("EMMC: Reset OK\n");
    #endif

    mmio_write(EMMC_CONTROL1, 
        mmio_read(EMMC_CONTROL1) | C1_CLK_INTLEN | C1_TOUNIT_MAX
    );
    wait_msec(10);

    if((r=sd_clk(400000))) return r;

    mmio_write(EMMC_INT_EN, 0xffffffff);
    mmio_write(EMMC_INT_MASK, 0xffffffff);

    sd_scr[0]=sd_scr[1]=sd_rca=sd_err=0;
    sd_cmd(CMD_GO_IDLE, 0);
    if(sd_err) return sd_err;

    sd_cmd(CMD_SEND_IF_COND, 0x000001AA);
    if(sd_err) return sd_err;

    cnt=6;
    r=0;
    while(!(r&ACMD41_CMD_COMPLETE) && cnt--){
        wait_cycles(400);
        
        r=sd_cmd(CMD_SEND_OP_COND, ACMD41_ARG_HC);
        #ifdef DEBUG
        printf("EMMC: CMD_SEND_OP_COND returned ");

        if(r&ACMD41_CMD_COMPLETE)
            printf("COMPLETE ");
        if(r&ACMD41_VOLTAGE)
            printf("VOLATGE ");
        if(r&ACMD41_CMD_CCS)
            printf("CCS ");
        printf("%x %x\n", r>>32, r);
        #endif
        if(sd_err!=SD_TIMEOUT && sd_err!=SD_OK){
            #ifdef DEBUG
            printf("EMMC: ERROR: ACMD41 returned error\n");
            #endif
            return sd_err;
        }
    }


    if(!(r&ACMD41_CMD_COMPLETE) || !cnt) return SD_TIMEOUT;
    if(!(r&ACMD41_VOLTAGE) || !cnt) return SD_ERROR;
    if(r&ACMD41_CMD_CCS) ccs=SCR_SUPP_CCS;

    sd_cmd(CMD_ALL_SEND_CID, 0);

    sd_rca=sd_cmd(CMD_SEND_REL_ADDR, 0);
    #ifdef DEBUG
    printf("EMMC: CMD_SEND_REL_ADDR returned %x %x\n", sd_rca>>32, sd_rca);
    #endif
    if(sd_err) return sd_err;

    if((r=sd_clk(25000000))) return r;

    sd_cmd(CMD_CARD_SELECT, sd_rca);
    if(sd_err) return sd_err;

    if(sd_status(SR_DAT_INHIBIT)) return SD_TIMEOUT;
    mmio_write(EMMC_BLKSIZECNT, (1<<16) | 8);
    sd_cmd(CMD_SEND_SCR, 0);
    wait_msec(1000000);
    if(sd_err) return sd_err;
    if(sd_int(INT_READ_RDY)) return SD_TIMEOUT;

    
    r=0;
    cnt=100000;
    while(r<2 && cnt){
        if(mmio_read(EMMC_STATUS) & SR_READ_AVAILABLE)
            sd_scr[r++]= mmio_read(EMMC_DATA);
        else
            wait_msec(1);
    }
    if(r!=2) return SD_TIMEOUT;
    if(sd_scr[0] & SCR_SD_BUS_WIDTH_4){
        sd_cmd(CMD_SET_BUS_WIDTH, sd_rca|2);
        if(sd_err) return sd_err;
        mmio_write(EMMC_CONTROL0, 
            mmio_read(EMMC_CONTROL0) | C0_HCTL_DWIDTH
        );
    }

    #ifdef DEBUG
    printf("EMMC: Supports ");
    if(sd_scr[0] & SCR_SUPP_SET_BLKCNT)
        printf("SET_BLKCNT ");
    if(ccs)
        printf("CCS ");
    printf("\n");
    #endif
    sd_scr[0] &= ~SCR_SUPP_CCS;
    sd_scr[0] |= ccs;
    return SD_OK;
}