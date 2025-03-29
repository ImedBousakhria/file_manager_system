#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <disk.h>

typedef struct {
    int used;              // 1 if used 0 if not
    int size;              // file size in bytes
    int *block;            // pointers to data blocks (multiple pointers to each block)
    int permissions;       // permits
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

// Inode allocationInode(Inode ino, int n_block){
//     int *tab= (int *)malloc(sizeof(int) * n_block);
    
// }