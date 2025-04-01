#include <stdio.h>
#include <stdlib.h>
#include "fs.h"

#define FS_FILE_NAME "fs_file_name.bin"

FileSystem fs_metadata;

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


int main (int argc, char *argv[]) {
    return EXIT_SUCCESS;
}