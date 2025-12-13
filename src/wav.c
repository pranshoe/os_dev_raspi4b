#include "wav.h"
#include "audio.h"
#include "kerio.h"

void wav_parser(unsigned char *data){
    if(!((data[0]=='R' && data[1]=='I' && data[2]=='F' && data[3]=='F') 
        && (data[8]=='W' && data[9]=='A' && data[10]=='V' && data[11]=='E'))){
            #ifdef DEBUG
            printf("WAV: Not a valid wav file");
            #endif
            return;
    }

    data += 4;
    int file_size=*(unsigned short*)data;
    #ifdef DEBUG
    printf("Size: %d\n", file_size);
    #endif

    data+=8;
    if(!(data[0]=='f' && data[1]=='m' && data[2]=='t' && data[3]==' ')){
        #ifdef DEBUG
        printf("Not a valid fmt chunk");
        #endif
        return;
    }

    data+=4;
    int chunk_size=*(unsigned short*)data;
    #ifdef DEBUG
    printf("FMT chunk size: %u\n", chunk_size);
    #endif

    data+=4;
    fmt_t fmt = *(fmt_t *)data;
    #ifdef DEBUG
    printf("Format: %x\n", fmt.format_tag);
    printf("Channels: %u\n", fmt.num_channels);
    printf("Sample Rate: %u\n", fmt.sample_rate);
    printf("Data Rate: %u\n", fmt.data_rate);
    printf("Data Block Size:%u\n", fmt.data_block_size);
    printf("Bit Depth: %u\n", fmt.bit_depth);
    printf("cbSize: %u\n", fmt.extra_size);
    printf("Actual Bit Depth: %u\n", fmt.true_bit_depth);
    printf("Channel Mask: %x\n", fmt.channel_mask);
    printf("Extended Format Tag: %x\n", fmt.ext_format_tag);
    #endif

    audio_set_sample_rate(fmt.sample_rate);

    data+=chunk_size;
    while(!(data[0]=='d' && data[1]=='a' && data[2]=='t' && data[3]=='a')){
        data+=4;
        data+=4 + *(unsigned int *)(data);
    }
    #ifdef DEBUG
    printf("Data Block Found\n");
    #endif
    data+=4;
    #ifdef DEBUG
    printf("Data Block Size %u\n",*(unsigned int*)data);
    #endif
    playaudio(data+4, *(unsigned int*)data, fmt.bit_depth/8);
}