#ifndef DISK_H 
#define DISK_H

#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define DISK_NAME "fs_vdisk.img"

int disk_init();
// int disk_read(int block_num, void *buf);
// int disk_write(int block_num, const void *buf); 
// void disk_format();  

#endif