#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "disk.h"




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
    if( new_inode_idx == n_inodes ){
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
    save_file_system();
    printf("File '%s' created successfully in directory :  '%s' .\n", filename, parent_path);
    

    return new_inode_idx;
}


/* Changing the actual content of a file aka writing in the last spot */
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

    FILE *disk_file = fopen("fs_vdisk.img", "r+b");
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

        // Seek the correct location of the right block
        fseek(disk_file, block_idx * BLOCK_SIZE + offset_in_last_block, SEEK_SET);

        // write the necessary amount of data 
        int writable_size = (remaining_size < (BLOCK_SIZE - offset_in_last_block)) ? remaining_size : (BLOCK_SIZE - offset_in_last_block);
        fwrite(data + data_offset, 1, writable_size, disk_file);


        // update counters
        remaining_size -= writable_size;
        data_offset += writable_size;
        last_block_index++;
        offset_in_last_block = 0;
    }

    fclose(disk_file);

    // update file size
    file_inode->size += size;

    // save changes
    save_file_system();

    printf("Successfully wrote %d bytes to file '%s'.\n", size, filename);
}

//  utility function to seek location based on offset calculated
void seek_to_location(FILE *disk_file, int block_idx, int offset_in_block) {
    fseek(disk_file, block_idx * BLOCK_SIZE + offset_in_block, SEEK_SET);
}

/* Reading files by block */
void read_from_file(const char *parent_path, const char *filename, char *buffer, int buffer_size) {
    
    // Find the directory index from provided path
    int parent_inode_idx = find_directory_index(parent_path);
    if (parent_inode_idx == -1) {
        printf("Path is not valid\n");
        return;
    }

    // check if file exists in the path, if not, do nothing
    int file_inode_idx = -1;
    Directory *parent_directory = &fs_metadata.directories[parent_inode_idx];
    for (int i = 0; i < parent_directory->num_entries; i++) {
        if (strcmp(parent_directory->entries[i].name, filename) == 0) {
            file_inode_idx = parent_directory->entries[i].inode_index;
            break;
        }
    }

    if (file_inode_idx == -1) {
        printf("File does not exist\n");
        return;
    }

    Inode *file_inode = &fs_metadata.inodes[file_inode_idx];

    // Actually reading the file data 
    int bytes_read = 0;
    int block_idx;
    int offset_in_block = file_inode->size % BLOCK_SIZE;

    FILE *disk_file = fopen("fs_vdisk.img", "rb");
    if (!disk_file) {
        printf("Error: Could not open disk file.\n");
        return;
    }

    for (int i = 0; i < 30 && bytes_read < buffer_size; i++) {
        block_idx = file_inode->blocks[i];
        if (block_idx == -1) {
            break; // No more blocks, exit
        }

        // Seek to the correct block location
        seek_to_location(disk_file, block_idx, 0);

        // Read data from the block into the buffer
        int read_size = (buffer_size - bytes_read < BLOCK_SIZE) ? buffer_size - bytes_read : BLOCK_SIZE;
        printf("what to read: \t%d", read_size);
        fread(buffer + bytes_read, 1, read_size, disk_file);
        printf("trying to print buf from read func  %s", buffer);


        
        bytes_read += read_size;
        offset_in_block = 0; // no offset needed for next new blocks
    }

    fclose(disk_file);

    // error case 1: not enough data has been retrieved
    if (bytes_read < buffer_size) {
        printf("Warning: Only %d bytes were read from file '%s'.\n", bytes_read, filename);
    }

    printf("Successfully read %d bytes from file '%s'.\n", bytes_read, filename);
}




/*
 *    change the used to 0 
 *    go back to the dir and delete it from an entry with delete_entry(int index_entry)
 *    free blocks when deleting files: the bitmaps = 0   
*/
void delete_inode(int inode_index){
    Inode *inode = &fs_metadata.inodes[inode_index];
    inode->used = 0;
    // remove from parent directory
    int parent_index = inode->parent_index;
    delete_entry(inode_index, parent_index, 1);
    //we have to change the bitmaps in the blocks
    int nb_blocks = ((inode->size) + BLOCK_SIZE -1 ) / BLOCK_SIZE;
    for(int i=0; i< nb_blocks; i++){
        int bit_map_index = inode->blocks[i];
        FileSystem *fs= &fs_metadata;
        fs->free_blocks[bit_map_index] = 0;
    }
    // we have to save the fs_metadata after and store it in the disk
    save_file_system();
}