#include "mbox.h"
#include "kerio.h"
#include "framebuffer.h"
#include "stdlib.h"
#include "delay.h"

unsigned int width, height, pitch, isrgb, bytes_per_pixel, char_width, char_height, chars_x, chars_y;
unsigned char *fb;
psf_t *font;
extern volatile unsigned char _binary_res_font_psf_start;

pixel_t BLACK = {0x00, 0x00, 0x00};
pixel_t WHITE = {0xff, 0xff, 0xff};

void framebuffer_init(){

    wait_msec(100000);

    mbox[0] = 35*4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_SETPHYWH;
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1920;
    mbox[6] = 1080;

    mbox[7] = MBOX_TAG_SETVIRTWH;
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1920;
    mbox[11] = 1080;

    mbox[12] = MBOX_TAG_SETVIRTOFFSET;
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;
    mbox[16] = 0; 

    mbox[17] = MBOX_TAG_SETDEPTH;
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;

    mbox[21] = MBOX_TAG_SETPIXORD;
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1;

    mbox[25] = MBOX_TAG_ALLOCFB;
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;
    mbox[29] = 0;

    mbox[30] = MBOX_TAG_GETPTCH;
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;

    mbox[34] = MBOX_TAG_LAST;


    if(mbox_call(MBOX_CH_PROP) && mbox[28]!=0){
        mbox[28]&=0x3FFFFFFF;
        width = mbox[5];
        height = mbox[6];
        pitch = mbox[33];
        isrgb = mbox[24];
        bytes_per_pixel = mbox[20]/8;
        fb=(void *)((unsigned long)mbox[28]);

        // puts("Screen resolution set to 1024*768");
    } else{
        // puts("Unable to set screen resolution to 1024*768\n");
    }

    font = (psf_t*)&_binary_res_font_psf_start;
    char_width = width/font->width;
    char_height = height/font->height-1;

}

void write_pixel(unsigned int x, unsigned int y, pixel_t *pixel){
    void *location = fb + y*pitch + x*bytes_per_pixel;
    memcpy(location, pixel, bytes_per_pixel);
}

void framebuffer_putc(char c){

    unsigned char *glyph = (unsigned char*)&_binary_res_font_psf_start + font->headersize + ((unsigned char)c<=font->num_glyphs ? c : 0)*font->bytes_per_glyph;

    if(char_width<=chars_y){
        int y=0;
        for(; y<chars_y-1; y++)
            memcpy(fb + y*pitch*font->height, fb + (y+1)*pitch*font->height, pitch*font->height);

        memcpy(fb + y*pitch*font->height, 0, pitch*font->height);
        chars_y--;
    }

    if(c=='\n'){
        chars_x=0;
        chars_y++;
    } else if(c=='\r'){
        chars_x=0;
    } 
    else if(c=='\t'){
        if(chars_x+4 <= char_width) printf("    ");
        else printf("\n");
    }else{
        for(int h=0; h<font->height; h++){
            for(int w=0; w<font->width; w++){
                unsigned int mask = 1<<(font->width-w);
                if(glyph[h] & mask)
                    write_pixel(chars_x*font->width + w, chars_y*font->height + h, &WHITE);
                else 
                    write_pixel(chars_x*font->width + w, chars_y*font->height + h, &BLACK);
            }
        }
        chars_x++;
    }

    if(chars_x > char_width){
        chars_x=0;
        chars_y++;
    }


}
