#ifndef FS_H
#define FS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <disk.h>

#define MAX_NAME_LENGTH 32
#define NUM_BLOCKS 1024  // Adjust as needed
#define MAX_ENTRIES_PER_DIR 128  // Example max entries in a directory
#define MAX_DIR 15 // max directories in our fs

#define MAX_PATH_LENGTH 256  // path length used in cwd
char current_working_directory[MAX_PATH_LENGTH] = "/"; 
int cwd_index = 0;

typedef struct {
    int isfile;
    char name[MAX_NAME_LENGTH];  // File/Directory name
    int inode_index;             // Points to the inode of file/directory
} DirectoryEntry;

typedef struct {
    int parent_index;
    int num_entries;                   // Number of files/directories
    DirectoryEntry entries[MAX_ENTRIES_PER_DIR]; // Directory contents
    /* first entry in entries would be the directory itself */
    /* second one's like {.. : parent dir index} */
} Directory;

typedef struct {
    int used;              // 1 if used, 0 if free
    int size;              // File size in bytes (0 for directories)
    int permissions;       // UNIX-style permissions
    int owner_indx;
    int blocks[30];      
    int parent_index;    
} Inode;

typedef struct{
    char * name;
    int groupe;
} User;

typedef struct {
    int nb_inodes;     // keep track of how many inodes did we fill (not all of them are used)
    int nb_directories;   // keep track of how many dir did we fill 
    Inode inodes[NUM_BLOCKS];  // Inode table
    Directory directories[MAX_DIR];   // max of the directories that you can ever create
    uint8_t free_blocks[NUM_BLOCKS];  // Bitmap for free blocks
    User users[3];
} FileSystem;


extern FileSystem fs_metadata;

void save_file_system();
int load_file_system();
void init_fs();
void mount_fs();
int disk_init();

#endif
