#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

// Function to find a free inode


int find_free_inode() {
    int nb_inodes = fs_metadata.nb_inodes;
    if (nb_inodes >= NUM_BLOCKS) {
        return -1; // No free inodes available
    }
    // we look in the not used ones (deleted)
    for (int i = 0; i < nb_inodes; i++) {
        if (!fs_metadata.inodes[i].used) {
            return i;
        }
    }
    // don't forget to increament the nb_inodes when you create the inode if i == nb_inodes + 1 
    return nb_inodes; 
}

/**
 * function to find the index of the last directory in a path
 * returns -1 if not found
 */

int find_directory_index(const char *path) {
    // Path copy
    char *path_copy = strdup(path);  

    // Tokenize directories composing the path
    char *token = strtok(path_copy, "/"); 
    
    // start from root directory
    int current_dir_index = 0;

    while (token != NULL) {
        int found = 0;

        // Unique loop, because we're jump searching (not sequencial)
        for (int i = 0; i < fs_metadata.directories[current_dir_index].num_entries; i++) {
            if (strcmp(fs_metadata.directories[current_dir_index].entries[i].name, token) == 0) {

                current_dir_index = fs_metadata.directories[current_dir_index].entries[i].inode_index;
                found = 1;
                break;
            }
        }

        // a path component is not found in its parent = invalid path
        if (!found) {
            free(path_copy);
            return -1; // path doesn't exist
        }

        // Move to the next token (next directory in path)
        token = strtok(NULL, "/");
    }

    free(path_copy);
    return current_dir_index;
}


/*
    deleting an entry from the directory 
    we put the last entry in it's place and decrease num_entries

*/ 
int delete_entry(){
     
}

/*
    change the used to 0 
    go back to the dir and delete it from an entry with delete_entry(int index_entry)
*/
int delete_inode(int inode_index){
    
}

/**
 * check if the file/directory existes in the dir given 
 */
int entry_exists(char *name, int dir_index, int isfile){
    Directory dir =fs_metadata.directories[dir_index];
    for(int i=0; i < dir.num_entries; i++){
        if(strcmp(dir.entries[i].name, name) == 0 && dir.entries[i].isfile == isfile){
            return 1;
        }
    }
    return 0;
}



/**
 * function to create an empty new file 
 *  */ 
void create_file(const char *filename, const char* parent_path, int user_index, int permission) {
    
    // get out if parent_path does not exist 
    int dir_idx = find_directory_index(parent_path);
    if (dir_idx == -1) {
        printf("Error: directory path not found.\n");
        return;
    }
    
    //check if there is a file with the same name in the dir 

    if(entry_exists(filename, dir_idx, 1)){
        printf("Error: a file with the same name already exists");
        return;
    }

    Directory *parent_directory = &fs_metadata.directories[dir_idx];

    // Check if there's space for a new entry in the directory
    if (parent_directory->num_entries >= MAX_ENTRIES_PER_DIR) {
        printf("Error: Directory is full.\n");
        return;
    }

    // Find a free inode for the new file
    int new_inode_idx = find_free_inode();
    //check for free inodes in the Inodes table in FileSystem 
    if (new_inode_idx == -1) {
        printf("Error: No free inodes available.\n");
        return;
    }

    // if we created a new one and we didn't overwrite a non used inode
    int n_inodes = fs_metadata.nb_inodes;
    if(new_inode_idx == n_inodes ){
        // update the nb_inodes
        FileSystem *fs =  &fs_metadata;
        fs->nb_inodes = n_inodes + 1;
    }

    // Init the new file inode
    Inode *new_inode = &fs_metadata.inodes[new_inode_idx];
    new_inode->used = 1;
    new_inode->size = 0;
    new_inode->permissions = permission;
    new_inode->owner_indx = user_index;
    new_inode->parent_index = dir_idx; // Set parent directory

 
    DirectoryEntry *entry = &parent_directory->entries[parent_directory->num_entries++];
    strcpy(entry->name, filename);
    entry->inode_index = new_inode_idx;
    entry->isfile = 1; // Mark as file

    printf("File '%s' created successfully in directory :  '%s' .\n", filename, parent_path);
}


/**
 * Function to create a new directory
 */
void create_directory(const char *dirname, const char* parent_path, int user_index, int permission) {
    // Get out if parent_path does not exist
    int dir_idx = find_directory_index(parent_path);

    if (dir_idx == -1) {
        printf("Error: Directory path not found.\n");
        return;
    }

    // Check if a directory with the same name exists in the parent dir
    if (entry_exists(dirname, dir_idx, 0)) {
        printf("Error: A directory with the same name already exists.\n");
        return;
    }

    Directory *parent_directory = &fs_metadata.directories[dir_idx];

    // Check if there's space for a new entry in the directory
    if (parent_directory->num_entries >= MAX_ENTRIES_PER_DIR) {
        printf("Error: The parent directory is full.\n");
        return;
    }

    // Find a free directory slot
    if (fs_metadata.nb_directories >= MAX_DIR) {
        printf("Error: No free directory slots available.\n");
        return;
    }

    // Initialize the new directory at the last available index
    int index = fs_metadata.nb_directories;
    Directory *new_dir = &fs_metadata.directories[index];
    new_dir->parent_index = dir_idx;
    new_dir->num_entries = 0;

    // Add "." and ".." entries for self-reference and parent reference
    DirectoryEntry self_entry;
    self_entry.isfile = 0;
    self_entry.inode_index = index;
    strcpy(self_entry.name, ".");

    DirectoryEntry parent_entry;
    parent_entry.isfile = 0;
    parent_entry.inode_index = dir_idx;
    strcpy(parent_entry.name, "..");

    new_dir->entries[new_dir->num_entries++] = self_entry;
    new_dir->entries[new_dir->num_entries++] = parent_entry;

    // Increase the number of directories
    fs_metadata.nb_directories++;

    // Add the new directory entry to the parent directory
    DirectoryEntry *entry = &parent_directory->entries[parent_directory->num_entries++];
    strcpy(entry->name, dirname);
    entry->inode_index = index;
    entry->isfile = 0; // Mark as a directory

    printf("Directory '%s' created successfully in directory path: '%s'.\n", dirname, parent_path);
}
