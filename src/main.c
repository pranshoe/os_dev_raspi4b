#include "uart.h"
#include "kerio.h"
#include "framebuffer.h"
#include "emmc.h"
#include "fat.h"
#include "audio.h"

extern unsigned char _end;

void kernel_main(){
    uart_init();
    framebuffer_init();
    printf("pranshoe's first kernel\n");   
    
    audio_init();
    if(sd_init()==SD_OK){
        if(fat_getpartition()){
            fat_listdir();
        }
    }

}