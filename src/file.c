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
    // don't forget to increament the nb_inodes when you create the inode if i == nb_inodes+1
    return nb_inodes+1; // No free inodes available
}

// Function to find the directory index corresponding to an inode


// change this imed !!!!!!!!!! it's finding the path if it exists (take a string i think then looks in the entry names and is dir) then just return the index in the 
// directory table in the file system 


int find_directory_index(int inode_idx) {
    for (int i = 0; i < MAX_DIR; i++) {
        if (fs_metadata.directories[i].num_entries > 0) {
            for (int j = 0; j < fs_metadata.directories[i].num_entries; j++) {
                if (fs_metadata.directories[i].entries[j].inode_index == inode_idx) {
                    return i;  // Found the directory index
                }
            }
        }
    }
    return -1; // Directory not found
}
/*
    deleting an entry from the directory 
    we put the last entry in it's place and decrease num_entries

*/ 
int delete_entry(){
     
}

/*
    we have to replace it with the last one in the inodes table 
    go back to the dir and delete it from an entry with delete_entry(int index_entry)
*/
int delete_inode(int inode_index){

}

/**
 * check if the file existes in the dir given 
 */
int file_exists(char *filename, int dir_index){
    Directory dir =fs_metadata.directories[dir_index];
    for(int i=0; i < dir.num_entries; i++){
        if(strcmp(dir.entries[i].name, filename) && dir.entries[i].isfile){
            return 1;
        }
    }
    return 0;
}

// here i have to change the second param to be a path then we look for its index with the function
// for now the user is an int 
void create_file(const char *filename, char* parent_path, int user) {
    
    // get out if parent_path does not exist 
    int dir_idx = find_directory_index(parent_path);
    if (dir_idx == -1) {
        printf("Error: directory path not found.\n");
        return;
    }
    
    //check if there is a file with the same name in the dir 

    if(file_exists(filename, dir_idx)){
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
    if( new_inode_idx == (n_inodes +1)){
        // update the nb_inodes
        FileSystem *fs =  &fs_metadata;
        fs->nb_inodes = n_inodes+1;
    }

    // Initialize the new file inode
    Inode *new_inode = &fs_metadata.inodes[new_inode_idx];
    new_inode->used = 1;
    new_inode->size = 0;
    new_inode->permissions = 777;
    new_inode->parent_index = dir_idx; // Set parent directory

 
    DirectoryEntry *entry = &parent_directory->entries[parent_directory->num_entries++];
    strcpy(entry->name, filename);
    entry->inode_index = new_inode_idx;
    entry->isfile = 1; // Mark as file

    printf("File '%s' created successfully in directory :  '%s' .\n", filename, parent_path);
}