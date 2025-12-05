#ifndef MBOX_H
#define MBOX_H

#include "peripheral.h"

/*
    0:  Total Size in Bytes     (n+1)*4
    1: Request/Response code
    2: Tag Idenstifier
    3: Value Buffer Size        (How many Bytes of data are allocated for this tag's value)
    4: Request Code             (Requests: Size of data you're sending; Responses: Size of data GPU returned) 
    5+: Sending/Receiving Data
*/


extern volatile unsigned int mbox[36];

//Channels
enum{
    MBOX_CH_POWER=0,
    MBOX_CH_FB=1,
    MBOX_CH_VUART=2,
    MBOX_CH_VCHIQ=3,
    MBOX_CH_LEDS=4,
    MBOX_CH_BTNS=5,
    MBOX_CH_TOUCH=6,
    MBOX_CH_COUNT=7,
    MBOX_CH_PROP=8
};

//Tags
enum{
    MBOX_TAG_GETSERIAL=0x10004,
    MBOX_TAG_SETCLKRATE=0x38002,
    MBOX_TAG_SETPOWER=0x28001,
    MBOX_TAG_SETPHYWH=0x48003,
    MBOX_TAG_SETVIRTWH=0x48004,
    MBOX_TAG_SETVIRTOFFSET=0x48009,
    MBOX_TAG_SETDEPTH=0x48005,
    MBOX_TAG_SETPIXORD=0x48006,
    MBOX_TAG_ALLOCFB=0x40001,
    MBOX_TAG_GETPTCH=0x40008,
    MBOX_TAG_SETGPIOSTATE=0x38041,
    MBOX_TAG_LAST=0
};


enum{
    MBOX_BASE = (PERIPHERAL_BASE + VIDEOCORE_MBOX_OFFSET),
    MBOX_READ = (MBOX_BASE),
    MBOX_POLL = (MBOX_BASE + 0x10),
    MBOX_SENDER = (MBOX_BASE + 0x14),
    MBOX_STATUS = (MBOX_BASE + 0x18),
    MBOX_CONFIG = (MBOX_BASE + 0x1C),
    MBOX_WRITE = (MBOX_BASE + 0x20)
};

typedef enum{
    MBOX_REQUEST=0x0,
    MBOX_RESPONSE = 0x80000000
} buffer_req_res_code_t;

typedef union{
    struct {
    unsigned int reserved: 30;
    unsigned char empty: 1;
    unsigned char full:1;
    };
    unsigned int as_int;
} mail_status_t;

int mbox_call(unsigned char ch);

#endif