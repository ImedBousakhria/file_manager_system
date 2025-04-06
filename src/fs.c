#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "permissions.h"

#define FS_FILE_NAME "fs_vdisk2.img"

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


/**
 * for debugging
 * print the inodes in the table of inodes in the filesystem structor 
 */

void print_all_inodes(){
    for(int i=0; i < fs_metadata.nb_inodes; i++){
        Inode inode = fs_metadata.inodes[i];
        printf("inode identifier : %d \n", i);
        printf("parent index : %d \n", inode.parent_index);
        printf("used or not : %d \n", inode.used);
    }
} 
/**
 * for debugging 
 * print the directories in the table of directories 
 */
void print_all_dir(){
    for(int i=0; i < fs_metadata.nb_directories; i++){
        Directory dir = fs_metadata.directories[i];
        printf("the dir identifier : %d \n", i);
        printf("num_entries : %d \n", dir.num_entries);
        printf("the entries : \n");
        printf("\n***\n");

        for(int i=0; i< dir.num_entries; i++){
            DirectoryEntry entry = dir.entries[i];
            printf("entry index: %d \n", entry.inode_index);
            printf("the entry type : %d \n", entry.isfile);
            printf("name of entry : %s \n", entry.name);
        }
        printf("\n---------------------\n\n\n");

    }
}
void list_all_from_root() {
    printf("/\n");
    traverse_directory(0, 1);  // Start traversal (root is at index 0)
}


void traverse_directory(int dir_index, int depth) {
    Directory *dir = &fs_metadata.directories[dir_index];

    for (int i = 2; i < dir->num_entries; i++) {
        DirectoryEntry *entry = &dir->entries[i];

        // identation for better visuals
        for (int j = 0; j < depth; j++) {
            printf("-----");
        }

        if (entry->isfile) {
            printf("[FILE] %s\n", entry->name);
        } else {
            printf("[DIR]  %s/\n", entry->name);
            traverse_directory(entry->inode_index, depth + 1);  // Recurse into the children
        }
    }
}
