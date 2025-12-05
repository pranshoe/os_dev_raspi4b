#include "fat.h"
#include "emmc.h"
#include "kerio.h"
#include "stdlib.h"

extern unsigned char _end;

static unsigned int part_lba =0, root_sec;
static bpb_t bpb;
static unsigned int num_root_dir = 0;
static unsigned int bytes_per_sector = 0;

int fat_getpartition(void){
    unsigned char *mbr = &_end;
    // bpb_t bpb;

    if(sd_readblk(0, &_end, 1)){

        //Check Magic number
        if(mbr[510]!=0x55 || mbr[511]!=0xAA){
            #ifdef DEBUG
            printf("FAT: Error: Bad magic to MBR\n");
            #endif
            return 0;
        }

        if(mbr[0x1C2]!=0xE && mbr[0x1C2]!=0xC){
            #ifdef DEBUG
            printf("FAT: Error: Wrong partition type\n");
            #endif
            return 0;
        }

        #ifdef DEBUG
        printf("FAT: MBR disk identifier: %x\n", *((unsigned int *)((unsigned long)&_end + 0x1B8)));
        #endif

        // part_lba = *((unsigned int *)((unsigned long)&_end + 0x1C6));
        memcpy(&part_lba, mbr + 0x1C6, 4);

        #ifdef DEBUG
        printf("FAT: FAT partition starts at %x\n", part_lba);
        #endif

        if(!sd_readblk(part_lba, &_end, 1)){
            #ifdef DEBUG
            printf("FAT: Error: Unable to read boot record\n");
            #endif
            return 0;
        }

        memcpy(&bpb, &_end, sizeof(bpb));

        if(!(bpb.file_system_type[0]=='F' && bpb.file_system_type[1]=='A' && bpb.file_system_type[2]=='T') &&
            !(bpb.file_system_type2[0]=='F' && bpb.file_system_type2[1]=='A' && bpb.file_system_type2[2]=='T')){
                #ifdef DEBUG
                printf("FAT: Error: Unknown filesystem type\n");
                #endif
                return 0;
        }

        #ifdef DEBUG
        printf("FAT: Found type:  %s\n", bpb.sectors_per_fat_16 > 0 ? "FAT16" : "FAT32");
        #endif
        return 1;
    }
    return 0;
}


void fat_listdir(void){
    fatdir_t *dir;
    unsigned int s;
    memcpy(&num_root_dir, &_end + 17, 2);
    memcpy(&bytes_per_sector, &_end+11, 2);

    root_sec = ((bpb.sectors_per_fat_16 ?
        bpb.sectors_per_fat_16 :
        bpb.sectors_per_fat_32) * bpb.num_fat) + bpb.reserved_sectors;
    
    #ifdef DEBUG
    printf("FAT: Number of root directory entries: %x\n", num_root_dir);
    #endif

    s = num_root_dir* sizeof(fatdir_t);

    if(!bpb.sectors_per_fat_16)
        root_sec+=(bpb.root_cluster-2)*bpb.sectors_per_cluster;

    root_sec+=part_lba;

    #ifdef DEBUG
    printf("FAT: FAT root dir: %x\n",root_sec);
    #endif

    if(sd_readblk(root_sec, (unsigned char*)&_end, s/BLOCK_SIZE + 1)){
        printf("Attributes\t\tCluster\t\tSize\t\tName\n");
        dir=(fatdir_t*)&_end;
        for(;dir->name[0]!=0; dir++){

            if((dir->name[0]==0xE5) //Deleted Directories
                | (dir->attribute.read_only && dir->attribute.hidden && dir->attribute.system && dir->attribute.volume_label)) //Long Filename Entry
                continue;

            printf("%c%c%c%c%c%c",
                dir->attribute.read_only ? 'R' : '.',
                dir->attribute.hidden ? 'H' : '.',
                dir->attribute.system ? 'S' : '.',
                dir->attribute.volume_label ? 'L' : '.',
                dir->attribute.directory ? 'D' : '.',
                dir->attribute.archive ? 'A' : '.'
            );

            printf("\t\t");
            printf("%x", ((unsigned int)(dir->cluster_high))<<16 | dir->cluster_low);

            printf("\t\t");
            printf("%x", dir->size);

            s = dir->attribute.as_int;
            dir->attribute.as_int = '\0';
            printf("\t\t");
            printf("%s\n", dir->name);
            dir->attribute.as_int = s;

        }
    } else {
        printf("FAT: ERROR: Unable to load root directory\n");
    }
    

}

 char *fat_readfile(unsigned int cluster){
    unsigned int *fat32 = (unsigned int*)(&_end + bpb.reserved_sectors);
    unsigned int *fat16 = (unsigned short*)fat32;

    unsigned int data_sec, s;
    unsigned char *data, *ptr;

    printf("FAT: Reading file at %x", cluster);

    data_sec = ((bpb.sectors_per_fat_16 ? bpb.sectors_per_fat_16 : bpb.sectors_per_fat_32) * bpb.num_fat) + bpb.reserved_sectors;
    s = num_root_dir * sizeof(fatdir_t);
    if(bpb.sectors_per_fat_16>0)
        data_sec += (s+511)>>9;

    data_sec+=part_lba;

    s=sd_readblk(part_lba +1, (unsigned char *)&_end + 512, (bpb.sectors_per_fat_16? bpb.sectors_per_fat_16 : bpb.sectors_per_fat_32) + bpb.reserved_sectors);

    data = ptr= &_end + 512 + s;

    while(cluster>1 && cluster<(bpb.sectors_per_fat_16>0?0xFFF8:0xFFFFFF8)){
        sd_readblk((cluster-2)*bpb.sectors_per_cluster + data_sec, ptr, bpb.sectors_per_cluster);

        ptr+=bytes_per_sector * bpb.sectors_per_cluster;

        if(bpb.sectors_per_fat_16) memcpy(&cluster, &fat16 + cluster, 2);
        else memcpy(&cluster, &fat32 + cluster, 4);
    }
    return (char *)(data);
 }
