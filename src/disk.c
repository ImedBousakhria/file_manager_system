#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

#define DISK_FILE_NAME "fs_vdisk.img"

FILE *disk_file;

/**
 * Initializes the virtual disk.
 * - If the disk image exists, loads the filesystem.
 * - If it doesn't exist, creates the disk and initializes the filesystem.
 * Returns 0 on success, -1 on failure.
 */
int disk_init() {
    disk_file = fopen(DISK_FILE_NAME, "rb+");
    if (!disk_file) {
        printf("Creating new disk...\n");

        // Create a new disk file
        disk_file = fopen(DISK_FILE_NAME, "wb+");
        if (!disk_file) {
            perror("Error creating disk file");
            return -1;
        }

        // initialize and save the file system (the save is inside the init)
        init_fs();
    } else {
        // load file system from disk
        load_file_system();
    }
    return 0;
}

