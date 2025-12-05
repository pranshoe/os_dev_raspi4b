#ifndef FAT_H
#define FAT_H

typedef struct{
    char        jmp_ins[3];
    char        oem_identifier[8];
    unsigned short    bytes_per_sector;
    unsigned char     sectors_per_cluster;
    unsigned short    reserved_sectors;
    unsigned char     num_fat;
    unsigned char     num_root_dir0;
    unsigned char     num_root_dir1;
    unsigned short    total_sectors_16;
    unsigned char     media_descriptor;
    unsigned short    sectors_per_fat_16;
    unsigned short    sectors_per_track;
    unsigned short    num_heads;
    unsigned int    hidden_sectors;
    unsigned int    total_sectors_32;


    //Fat specific
    unsigned int    sectors_per_fat_32;
    unsigned short    flags;
    unsigned short    fat_version;
    unsigned int    root_cluster;
    unsigned char     padding_0[6];
    char        file_system_type[8];
    unsigned char     padding_1[20];
    char        file_system_type2[8];
} __attribute__((packed)) bpb_t;

typedef union {
    struct  {
        unsigned char     read_only:1;
        unsigned char     hidden:1;
        unsigned char     system:1;
        unsigned char     volume_label:1;
        unsigned char     directory:1;
        unsigned char     archive:1;
        unsigned char     reserved:2;
    };
    unsigned char as_int;
} __attribute__((packed)) fatdir_attrib_t;

typedef struct{
    char            name[8];
    char            ext[3];
    fatdir_attrib_t attribute;
    unsigned long        reserved;
    unsigned short        cluster_high;
    unsigned short        write_time;
    unsigned short        write_date;
    unsigned short        cluster_low;
    unsigned int        size;
} __attribute__((packed)) fatdir_t;

int fat_getpartition(void);
void fat_listdir(void);
char* fat_readfile(unsigned int cluster);

#endif