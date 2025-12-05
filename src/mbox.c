#include "mbox.h"
#include "uart.h"
#include "mmio.h"

volatile unsigned int __attribute__((aligned(16))) mbox[36];

int mbox_call(unsigned char ch){
    mail_status_t status;
    
    //Zeroes the last four bits of the address of the message and replaces them with the last four bits of the channel
    unsigned int msg_addr = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch & 0xF));

    //Wait until we can write to mailbox
    do{
        status.as_int = mmio_read(MBOX_STATUS);
    } while(status.full);
    
    mmio_write(MBOX_WRITE, msg_addr);
    
    //Wait for our message
    do{
        //Wait for response
        do{
            status.as_int = mmio_read(MBOX_STATUS);
        } while(status.empty);
        
    } while(msg_addr != mmio_read(MBOX_READ));
    
    if(mbox[1]==MBOX_RESPONSE)
        return 1;
    return 0;

}