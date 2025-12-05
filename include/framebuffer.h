#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

typedef struct{
    unsigned int magic;
    unsigned int version;
    unsigned int headersize;
    unsigned int flags;
    unsigned int num_glyphs;
    unsigned int bytes_per_glyph;
    unsigned int height;
    unsigned int width;
} psf_t;

typedef struct{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} pixel_t;

void framebuffer_init();
void framebuffer_putc(char c);

#endif