#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "permissions.h"

#define FS_FILE_NAME "fs_vdisk.img"

// Global variable for filesystem metadata
FileSystem fs_metadata;
extern char current_working_directory[MAX_PATH_LENGTH] = "/"; 
extern int cwd_index = 0;
/**
 * Saves the current filesystem state to disk.
 */
void save_file_system() {
    FILE *file = fopen(FS_FILE_NAME, "rb+");
    if (!file) {
        perror("Error opening disk file for writing");
        return;
    }

    fwrite(&fs_metadata, sizeof(FileSystem), 1, file);
    fclose(file);

    printf("File system saved successfully.\n");
}

/**
 * Loads the filesystem from disk.
 * Returns 1 if successful, 0 otherwise.
 */
int load_file_system() {
    FILE *file = fopen(FS_FILE_NAME, "rb");
    if (!file) {
        printf("Error: trying to load the filesystem but the disk image does not exist");
        return 0; // Disk image doesn't exist
    }

    size_t read_items = fread(&fs_metadata, sizeof(FileSystem), 1, file);
    fclose(file);

    if (read_items != 1) {
        printf("Error reading filesystem data.\n");
        return 0;
    }

    printf("File system loaded successfully.\n");
    return 1;
}

/**
 * Initializes a new filesystem.
 * - Creates a root directory.
 * - Initializes metadata.
 */
void init_fs() {
    memset(&fs_metadata, 0, sizeof(FileSystem));

    FileSystem *fs = &fs_metadata;

    // // Initialize users
    // strcpy(fs->users[0].name, "feryel");
    // fs->users[0].groupe = 1;
    
    // strcpy(fs->users[1].name, "imed");
    // fs->users[1].groupe = 1;
    
    // strcpy(fs->users[2].name, "dyhia");
    // fs->users[2].groupe = 2;
    User* dyhia = &fs_metadata.users[0];
    dyhia->name = "dyhia";
    dyhia->groupe = GROUP_ADMIN;
    
    // Feryel - admin (groupe 0)
    User* feryel = &fs_metadata.users[1];
    feryel->name = "feryel";
    feryel->groupe = GROUP_ADMIN;
    
    // Imad - utilisateur standard (groupe 1)
    User* imad = &fs_metadata.users[2];
    imad->name = "imad";
    imad->groupe = GROUP_USER;

    // mark all blocks as free
    memset(fs->free_blocks, 0, NUM_BLOCKS);

    // set initial inode count
    fs->nb_inodes = 0;

    // Create the root directory
    fs->nb_directories = 1;
    Directory *root = &fs->directories[0];
    //the parent is itself
    root->parent_index = 0;
    root->num_entries = 0;

    // Add "." entry (self-reference)
    DirectoryEntry self_entry;
    self_entry.isfile = 0;
    self_entry.inode_index = 0;
    strcpy(self_entry.name, ".");
    root->entries[root->num_entries++] = self_entry;

    // Add ".." entry for root (points to itself)
    DirectoryEntry parent_entry;
    parent_entry.isfile = 0;
    parent_entry.inode_index = 0; // Root points to itself
    strcpy(parent_entry.name, "..");
    root->entries[root->num_entries++] = parent_entry;


    printf("File system initialized successfully.\n");

    // save the initialized filesystem
    save_file_system();
}

/**
 * mounts the filesystem:
 * - If the disk exists, loads it.
 * - Otherwise, initializes a new filesystem.
 */
void mount_fs() {
    if (!load_file_system()) {
        printf("Creating new file system...\n");
        init_fs();
    }
}
