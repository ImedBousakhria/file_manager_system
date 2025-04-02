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
            DirectoryEntry *entry = &fs_metadata.directories[current_dir_index].entries[i];
            if (strcmp(entry->name, token) == 0  && !entry->isfile ) {
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
        if(strcmp(dir.entries[i].name, name) && (dir.entries[i].isfile == isfile)){
            return 1;
        }
    }
    return 0;
}



// here i have to change the second param to be a path then we look for its index with the function
// for now the user is an int 
int create_file(const char *filename, char* parent_path, int user_index) {
    
    // get out if parent_path does not exist 
    int dir_idx = find_directory_index(parent_path);
    if (dir_idx == -1) {
        printf("Error: directory path not found.\n");
        return;
    }
    
    // check if there is a file with the same name in the dir 

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
    if( new_inode_idx == (n_inodes +1)){
        // update the nb_inodes
        FileSystem *fs =  &fs_metadata;
        fs->nb_inodes = n_inodes + 1;
    }

    // Init the new file inode
    Inode *new_inode = &fs_metadata.inodes[new_inode_idx];
    new_inode->used = 1;
    new_inode->size = 0;
    new_inode->permissions = 777;
    new_inode->owner_indx = user_index;
    new_inode->parent_index = dir_idx; // Set parent directory

 
    DirectoryEntry *entry = &parent_directory->entries[parent_directory->num_entries++];
    strcpy(entry->name, filename);
    entry->inode_index = new_inode_idx;
    entry->isfile = 1; // Mark as file

    printf("File '%s' created successfully in directory :  '%s' .\n", filename, parent_path);
    return new_inode_idx;
}


void write_to_file(const char *parent_path, const char *filename, const char *data){
    // find the directory index from provided path
    int parent_inode_idx = find_directory_index(parent_path);
    int file_inode_idx;
    if (parent_inode_idx == -1){
        printf("Path is not valid");
        return ;
    }

    // check if file exists in the path, if not, create it
    if(!entry_exists(filename, parent_inode_idx, 1)){
        printf("File does not exist");
        file_inode_idx = create_file(filename, parent_path, 1);
    }
    

    // allocate blocks to write DATA from input
    int size = strlen(data);
    
    Inode *file_inode = &fs_metadata.inodes[file_inode_idx];

    int file_size = file_inode->size;
    int last_block_index = file_size / BLOCK_SIZE;
    int offset_in_last_block = file_size % BLOCK_SIZE;

    int remaining_size = size;
    int data_offset = 0;

    FILE *disk_file = fopen("disk.img", "r+b");
    if (!disk_file) {
        printf("Error: Could not open disk file.\n");
        return;
    }

    // Actually writing 
    while (remaining_size > 0) {
        int block_idx = -1;

        // use last block if it's not 100% filled
        if (last_block_index < 30 && file_inode->blocks[last_block_index] != 0) {
            int used_space_in_last_block = file_inode->size % BLOCK_SIZE;
    
            if (used_space_in_last_block < BLOCK_SIZE) {
                // There's still space in the last block, so continue writing there
                block_idx = file_inode->blocks[last_block_index];
            }
        } 
        // sinon, allocate a new block
        else {
            for (int j = 0; j < NUM_BLOCKS; j++) {
                if (fs_metadata.free_blocks[j] == 0) {
                    fs_metadata.free_blocks[j] = 1;
                    file_inode->blocks[last_block_index] = j;
                    block_idx = j;
                    break;
                }
            }
        }

        if (block_idx == -1) {
            printf("Error: No space left on disk.\n");
            fclose(disk_file);
            return;
        }

        // Seek to the correct position in the block
        fseek(disk_file, block_idx * BLOCK_SIZE + offset_in_last_block, SEEK_SET);

        // Write the necessary amount of data
        int writable_size = (remaining_size < (BLOCK_SIZE - offset_in_last_block)) ? remaining_size : (BLOCK_SIZE - offset_in_last_block);
        fwrite(data + data_offset, 1, writable_size, disk_file);

        // Update counters
        remaining_size -= writable_size;
        data_offset += writable_size;
        last_block_index++;
        offset_in_last_block = 0; // Only the first block might have an offset
    }

    fclose(disk_file);

    // Update file size
    file_inode->size += size;

    // Save changes
    save_file_system();

    printf("Successfully wrote %d bytes to file '%s'.\n", size, filename);
}
