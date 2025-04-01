#include <stdio.h>
#include <stdlib.h>
#include "fs.h"

#define FS_FILE_NAME "fs_file_name.bin"



void save_file_system() {
    FILE *file = fopen(FS_FILE_NAME, "wb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    // Write the entire FileSystem struct
    fwrite(&fs_metadata, sizeof(FileSystem), 1, file);
    fclose(file);
    printf("File system saved successfully.\n");
}
/*
    this function is to initialize the fs struct
    at the very first of of everything then we store it and
    save and store it each time we change it
*/ 
void init_fs(){

    FileSystem *fs = &fs_metadata;
    fs->nb_inodes= 0;
    fs->nb_directories = 0;

}

int main (int argc, char *argv[]) {
    return EXIT_SUCCESS;
}