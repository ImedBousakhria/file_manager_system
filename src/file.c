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
    return nb_inodes + 1; 
}

// Function to find the directory index corresponding to an inode


// change this imed !!!!!!!!!! it's finding the path if it exists (take a string i think then looks in the entry names and is dir)
// then just return the index in the directory table in the file system 


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

// int find_directory_index(char* path) {
//     for (int i = 0; i < MAX_DIR; i++) {
//         // only check the first entry since it's the one containing the dir name
//                 if (strcmp(fs_metadata.directories[i].entries[1].name, path)) {
//                     return i;  // Found the directory index
//                 }
//     }
//     return -1; // Directory not found
// }


// here i have to change the second param to be a path then we look for its index with the function
// for now the user is an int 
void create_file(const char *filename, int path, int user) {
    
    // parent inode is an index in directories
    Directory *parent_inode = &fs_metadata.directories[find_directory_index(path)].parent_index;


    if (!parent_inode){
        return -1;
    }

    // Find a free inode for the new file
    int new_inode_idx = find_free_inode();

    if (new_inode_idx == -1) {
        printf("Error: No free inodes available.\n");
        return;
    }

    // if we created a new one and we didn't write over a non used inode
    int n_inodes = fs_metadata.nb_inodes;
    if( new_inode_idx == (n_inodes +1)){
        // update the nb_inodes
        FileSystem *fs =  &fs_metadata;
        fs->nb_inodes = n_inodes + 1;
    }

    // Init the new file inode
    Inode *new_inode = &fs_metadata.inodes[new_inode_idx];
    new_inode->used = 1;
    new_inode->size = 0;
    new_inode->permissions = 0777;
    new_inode->parent_index = parent_inode; // Set parent directory idx

    // Update inode count
    fs_metadata.nb_inodes++;

    // Find the directory structure that corresponds to parent_inode_idx
    int dir_idx = find_directory_index(path);
    if (dir_idx == -1) {
        printf("Error: Parent directory not found.\n");
        return;
    }

    Directory *parent_directory = &fs_metadata.directories[dir_idx];

    // Check if there's space for a new entry
    if (parent_directory->num_entries >= MAX_ENTRIES_PER_DIR) {
        printf("Error: Directory is full.\n");
        return;
    }

    // Create a new directory entry
    DirectoryEntry *entry = &parent_directory->entries[parent_directory->num_entries++];
    strcpy(entry->name, filename);
    entry->inode_index = new_inode_idx;
    entry->isfile = 1; // Mark as file

    printf("File '%s' created successfully in directory (inode %d).\n", filename, path);
}


