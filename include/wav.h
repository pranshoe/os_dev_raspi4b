#ifndef WAV_H
#define WAV_H

typedef struct{
    unsigned short format_tag;
    unsigned short num_channels;
    unsigned int sample_rate;
    unsigned int data_rate;
    unsigned short data_block_size;
    unsigned short bit_depth;
    unsigned short extra_size;
    unsigned short true_bit_depth;
    unsigned int channel_mask;
    unsigned short ext_format_tag;
} __attribute__((__packed__)) fmt_t;

void wav_parser(unsigned char* data);

#endif