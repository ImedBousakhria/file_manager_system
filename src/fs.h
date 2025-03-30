#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <disk.h>

typedef struct {
    int isDirectory;       // 1 = Directory, 0 = File
    int used;             // 1 if used, 0 if free
    int size;             // File size in bytes
    int permissions;      // UNIX-style permissions

    int *blocks;          // For files: dynamically allocated block pointers

    struct {              
        int num_entries;      // How many files/subdirectories exist
        DirectoryEntry *entries;  // Dynamic list of directory contents
    } directory;

} Inode;

typedef struct {
    char name[32];         // file name
    int inode_index;       // take to inode of the file in inode table
} DirectoryEntry;

typedef struct {
    Inode inodes[NUM_BLOCKS];              // inode table
    DirectoryEntry root_directory[NUM_BLOCKS]; // root directory
    uint8_t free_blocks[NUM_BLOCKS];      // bitmap we'll see if we keep it
} FileSystem;
