#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "disk.h"
#include "permissions.h"
#include "utils.h"
#include "folder.h"



// for now the user is an int 
int create_file(const char *filename, char* parent_path, int user_index, int permissions) {
    
    // get out if parent_path does not exist 
    int dir_idx = find_directory_index(parent_path);
    if (dir_idx == -1) {
        printf("Error: directory path not found.\n");
        return -1;
    }
    
    // check if there is a file with the same name in the dir 

    if(entry_exists(filename, dir_idx, 1) > -1){
        printf("Error: a file with the same name already exists");
        return -1;
    }

    Directory *parent_directory = &fs_metadata.directories[dir_idx];

    // Check if there's space for a new entry in the directory
    if (parent_directory->num_entries >= MAX_ENTRIES_PER_DIR) {
        printf("Error: Directory is full.\n");
        return -1;
    }

    // Find a free inode for the new file
    int new_inode_idx = find_free_inode();
    //check for free inodes in the Inodes table in FileSystem 
    if (new_inode_idx == -1) {
        printf("Error: No free inodes available.\n");
        return -1;
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
    new_inode->permissions = permissions;
    new_inode->owner_indx = user_index;
    new_inode->parent_index = dir_idx; // Set parent directory
    for (int i = 0; i < 30; i++) {
        new_inode->blocks[i] = -1;
    }

 
    DirectoryEntry *entry = &parent_directory->entries[parent_directory->num_entries++];
    strcpy(entry->name, filename);
    entry->inode_index = new_inode_idx;
    entry->isfile = 1; // Mark as file
    save_file_system();
   printf("File '%s' created successfully in directory :  '%s' .\n", filename, parent_path);
    

    return new_inode_idx;
}


/* Changing the actual content of a file aka writing in the last spot */
void write_to_file(const char *parent_path, const char *filename, const char *data, int user_index){
    // find the directory index from provided path
    int parent_inode_idx = find_directory_index(parent_path);
    int file_inode_idx;
    if (parent_inode_idx == -1){
        printf("Path is not valid");
        return ;
    }

    // check if file exists in the path, if not, create it
    if(entry_exists(filename, parent_inode_idx, 1) == -1){
        printf("File does not exist");
        file_inode_idx = create_file(filename, parent_path, user_index, 777);
    }
    

    // allocate blocks to write DATA from input
    int size = strlen(data);
    
    Inode *file_inode = &fs_metadata.inodes[file_inode_idx];

    int file_size = file_inode->size;
    int last_block_index = file_size / BLOCK_SIZE;
    int offset_in_last_block = file_size % BLOCK_SIZE;

    int remaining_size = size;
    int data_offset = 0;

    FILE *disk_file = fopen("fs_vdisk2.img", "r+b");
    if (!disk_file) {
        printf("Error: Could not open disk file.\n");
        return;
    }


    int permit = check_permission(fs_metadata.users[user_index].name, file_inode_idx, 2);
    if (permit != 1){
        printf("User doesn't have the permissions to write in this file");
        return;
    }

    // Actually writing 
    while (remaining_size > 0) {
        int block_idx = -1;

        // use last block if it's not 100% filled
        if (last_block_index < 30 && file_inode->blocks[last_block_index] != -1) {
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
    printf("file size after update is  %d\n", file_inode->size);

    // save changes
    save_file_system();

    printf("Successfully wrote %d bytes to file '%s'.\n", size, filename);
}

//  utility function to seek location based on offset calculated
void seek_to_location(FILE *disk_file, int block_idx, int offset_in_block) {
    fseek(disk_file, block_idx * BLOCK_SIZE + offset_in_block, SEEK_SET);
}

/* Reading files by block */
void read_from_file(const char *parent_path, const char *filename, char *buffer, int buffer_size, int user_index) {
    
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
    int permit = check_permission(fs_metadata.users[user_index].name, file_inode_idx, 4);
    if (permit != 1){
        printf("User doesn't have the permissions to read from this file");
        return;
    }

    Inode *file_inode = &fs_metadata.inodes[file_inode_idx];

    // Actually reading the file data 
    int bytes_read = 0;
    int block_idx;
    int offset_in_block = file_inode->size % BLOCK_SIZE;

    FILE *disk_file = fopen("fs_vdisk2.img", "rb");
    if (!disk_file) {
        printf("Error: Could not open disk file.\n");
        return;
    }

    for (int i = 0; i < 30 && bytes_read < buffer_size; i++) {
        block_idx = file_inode->blocks[i];
        if (block_idx == -1) {
            break; // No more blocks, exit
        }
        printf("data block index is %d", block_idx);

        // Seek to the correct block location
        seek_to_location(disk_file, block_idx, 0);

        // Read data from the block into the buffer
        int read_size = (buffer_size - bytes_read < BLOCK_SIZE) ? buffer_size - bytes_read : BLOCK_SIZE;
        fread(buffer + bytes_read, 1, read_size, disk_file);
        printf("trying to print buf from read func  %s\n", buffer);


        
        bytes_read += read_size;
        offset_in_block = 0; // no offset needed for next new blocks
    }
    for (int i = 0; i < bytes_read; i++) {
        printf("%02X ", (unsigned char)buffer[i]);
    }
    printf("\n");

    fclose(disk_file);

    // error case 1: not enough data has been retrieved
    if (bytes_read < buffer_size) {
        printf("Warning: Only %d bytes were read from file '%s'.\n", bytes_read, filename);
    }

    printf("Successfully read %d bytes from file '%s'.\n", bytes_read, filename);
}



char* read_full_file(const char *parent_path, const char *filename, int user_index) {
    int parent_inode_idx = find_directory_index(parent_path);
    if (parent_inode_idx == -1) {
        printf("Error: Invalid path\n");
        return NULL;
    }

    // Find file inode
    int file_inode_idx = -1;
    Directory *parent_directory = &fs_metadata.directories[parent_inode_idx];
    for (int i = 0; i < parent_directory->num_entries; i++) {
        if (strcmp(parent_directory->entries[i].name, filename) == 0) {
            file_inode_idx = parent_directory->entries[i].inode_index;
            printf("filename is %s\n", filename);
            break;
        }
    }

    if (file_inode_idx == -1) {
        printf("Error: File not found\n");
        return NULL;
    }

    int permit = check_permission(fs_metadata.users[user_index].name, file_inode_idx, 4);
    if (permit != 1){
        printf("User doesn't have the permissions to read from this file");
        return;
    }
   

    Inode *inode = &fs_metadata.inodes[file_inode_idx];

    for (int i = 0; i < 30; i++) {
        printf("%d ", inode->blocks[i]);
    }
    printf("\n");


    int file_size = inode->size;
    if (file_size == 0) {
        printf("Warning: File is empty\n");
        return strdup(""); // Return empty string
    }

    char *buffer = malloc(file_size + 1); // +1 for null terminator
    if (!buffer) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }

    FILE *disk_file = fopen("fs_vdisk2.img", "rb");

    if (!disk_file) {
        printf("Error: Could not open disk\n");
        free(buffer);
        return NULL;
    }

    int bytes_read = 0;
    for (int i = 0; i < 30 && bytes_read < file_size; i++) {
        int block_idx = inode->blocks[i];
        if (block_idx == -1) break;

        fseek(disk_file, block_idx * BLOCK_SIZE, SEEK_SET);
        printf("Seeking to block %d at position %ld\n", block_idx, ftell(disk_file));
        int to_read = (file_size - bytes_read < BLOCK_SIZE) ? (file_size - bytes_read) : BLOCK_SIZE;
        fread(buffer + bytes_read, 1, to_read, disk_file);
    
        printf("\n");
        bytes_read += to_read;
    }

    fclose(disk_file);

    buffer[file_size] = '\0'; // Null-terminate the string

    printf("Read %d bytes from file '%s'\n", bytes_read, filename);
    return buffer;
}

/*
 *    change the used to 0 
 *    go back to the dir and delete it from an entry with delete_entry(int index_entry)
 *    free blocks when deleting files: the bitmaps = 0   
*/
void delete_inode(int inode_index, int user_index){
    Inode *inode = &fs_metadata.inodes[inode_index]; 
    // remove from parent directory
    int parent_index = inode->parent_index;

    int permit1 = check_permission(fs_metadata.users[user_index].name, parent_index, 1);
    int permit2 = check_permission(fs_metadata.users[user_index].name, parent_index, 2);
     
    if ((!permit1) || (!permit2)){
        printf("User doesn't have the permissions to move this directory");
        return; 
    }

    delete_entry(inode_index, parent_index, 1);
    inode->used = 0;
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


/**
 * move a file from a dir to another 
 */

void move_file(const char* path, const char* des_path, int user_index) {
    // deviding the path of the file to the parent path and the path 
    char *lastSlash = strrchr(path, '/');
    char *directory;
    char *filename;
    if (lastSlash != NULL) {
        // Get filename
        filename = lastSlash + 1;

        // Temporarily cut the string to get the directory path
        *lastSlash = '\0'; // Replace '/' with null character

        directory = path;

        printf("Directory: %s\n", directory);
        printf("Filename: %s\n", filename);
    } else {
        printf("Error: No directory found in source path.\n");
        return;
    }
    int parent_dir_index = find_directory_index(directory);
    if(parent_dir_index == -1){
        printf("Error: the source path is not found");
        return;
    }

    int file_index = entry_exists(filename, parent_dir_index, 1);
    if(file_index == -1){
        printf("Error: the file is not found in the source path");
        return;
    }

    // verify if the destination path is good if not no need to continue 

    int dis_dir_index = find_directory_index(des_path);
    if(dis_dir_index == -1){
        printf("Error: the destination path is not found");
        return;
    }
     // we add it as an entry to the new dir path 
    int entry_index = add_entry(dis_dir_index, file_index, 1, filename);
    if(entry_index == -1){
        printf("Error: The distination Directory is full");
        return;
    }



    int permit1 = check_permission(fs_metadata.users[user_index].name, parent_dir_index, 1);
    int permit2 = check_permission(fs_metadata.users[user_index].name, parent_dir_index, 2);
    int permit3 = check_permission(fs_metadata.users[user_index].name, dis_dir_index, 1);
    int permit4 = check_permission(fs_metadata.users[user_index].name, dis_dir_index, 2);

    if ((!permit1) || (!permit2) || (!permit3) || (!permit4)){
        printf("User doesn't have the permissions to move this directory");
        return;
    }


    Inode *inode = &fs_metadata.inodes[file_index];
    // change the parent index in the inode
    inode->parent_index = dis_dir_index;
    

    //in the parent dir we delete the entry of the file 
    delete_entry(file_index, parent_dir_index, 1);
    // saving the file system
    save_file_system();
}
