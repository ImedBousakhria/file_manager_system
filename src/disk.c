#include <stdio.h>
#include <stdlib.h>
#include <disk.h>
#include "fs.h"

FILE *disk_file;
FileSystem fs_metadata;

int disk_init() {
    if(!fopen('fs_vdisk.img', 'rb+')) {
        disk_file = fopen('fs_vdisk.img', 'wb+');
        
        if (!disk_file) return -1;
        // treat
    } else {
       fread(&fs_metadata, sizeof(FileSystem) , 1, disk_file);
    }
    return 0;
}

