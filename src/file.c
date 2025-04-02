#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "disk.h"
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
    // don't forget to increament the nb_inodes when you create the inode if i == nb_inodes  
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

        // unique loop, because we're jump searching (not sequencial)
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
    put the last entry in it's place and decrease num_entries if not 0 after the deleting
    no need to change any indexes here because you just changed 
*/ 
void delete_entry(int inode_index, int parent_index, int type){
        
        int entry_index = find_index_entry(inode_index, parent_index, type);
        if(entry_index == -1){
            printf("Error: The inode_index does not exist in the parent entries");
            return;
        }

        Directory *dir = &fs_metadata.directories[parent_index];
        int num_entries = dir->num_entries;
        // we put the last one in it's palce 
        if(entry_index < num_entries -1){
            dir->entries[entry_index] = dir->entries[num_entries -1];
        }

        dir->num_entries = num_entries -1;
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
}



/**
 * deleting a directory
 * here we get the last dir if it exist if not just make the num_directories-1 we keep the root
 * we recursivelly delete everything under it 
 */
void delete_dir(int dir_index){

    //before deleting a dir we have to make sure it's not the root
    if(dir_index == 0){
        printf("Error: You can't delete the root directory");
        return;
    }

    Directory *dir = &fs_metadata.directories[dir_index];
    Directory copy_dir = fs_metadata.directories[dir_index];
    // check if it's the last in the table then just decrement the num_directories
    int num_dir = fs_metadata.nb_directories;
    if(dir_index < num_dir -1){
        Directory last_dir = fs_metadata.directories[num_dir -1];
        // replace the deleted directory with the last one
        *dir = last_dir;
        // change the index of last dir in its dir parent
        int parent_index = last_dir.parent_index;
        
        Directory *parent_last_index = &fs_metadata.directories[parent_index];

        int entry_index = find_index_entry(num_dir -1, parent_index, 0);
        if(entry_index == -1){
            printf("Error: The Code is BAD !!! check if you're putting the values right in parent_index in DirectoryEntry, check if you're saving correctly");
            return;
        }

        // update the index of the last dir in his dir parent because we moved it 
        parent_last_index->entries[entry_index].inode_index = dir_index;
    }
    //decrementing the num_directories in the two cases
    FileSystem *fs = &fs_metadata;
    fs->nb_directories = num_dir -1; 
    // recursivlly delete everything under 
    
    for(int i=0; i< copy_dir.num_entries; i++){
        DirectoryEntry entry = copy_dir.entries[i];

        // skip "." and ".." entries to prevent infinite recursion

        if(strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0) {
        continue;
        }
        if(entry.isfile){
            delete_inode(entry.inode_index);
        } else {
            delete_dir(entry.inode_index);  // Only call for directories
        }
    }
    // save the file system in the disk 

}
/**
 * function that looks for the entry from the index in inodes/directories tables
 * and the dir parent index to find it's place in the entires 
 * returns the index in the entries 
 */
int find_index_entry(int index, int parent_index, int type){
    Directory dir_parent = fs_metadata.directories[parent_index];
    for(int i =0; i< dir_parent.num_entries; i++){
        if((dir_parent.entries[i].inode_index == index) && (dir_parent.entries[i].isfile == type)){
            return i;
        }     
    }
    return -1;
}
/**
 * check if the file/directory existes in the dir given 
 * 
 */
int entry_exists(char *name, int dir_index, int isfile){
    Directory dir =fs_metadata.directories[dir_index];
    for(int i=0; i < dir.num_entries; i++){
        if(strcmp(dir.entries[i].name, name) == 0 && dir.entries[i].isfile == isfile){
            if(isfile){
                /*check if inode used of not, we might have a deleted file with the same name*/
                Inode inode = fs_metadata.inodes[dir.entries[i].inode_index];
                if(inode.used){
                    return 1;
                }
            }else{
                return 1;
            }

            
        }
    }
    return 0;
}



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
        if (block_idx == 0) {
            break; // No more blocks, exit
        }

        // Seek to the correct block location
        seek_to_location(disk_file, block_idx, 0);

        // Read data from the block into the buffer
        int read_size = (buffer_size - bytes_read < BLOCK_SIZE) ? buffer_size - bytes_read : BLOCK_SIZE;
        fread(buffer + bytes_read, 1, read_size, disk_file);
        
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


const char* getcwd() {
    return current_working_directory;
}


// utility function that builds paths up to root
void get_full_path_from_index(int dir_index, char *output_path) {
    char temp_path[MAX_PATH_LENGTH] = "";
    char stack[15][MAX_NAME_LENGTH]; // Stack to store path parts
    int stack_top = 0;

    while (dir_index != 0) {  // Stop when reaching root
        Directory *dir = &fs_metadata.directories[dir_index];

        // Push directory name onto the stack
        strncpy(stack[stack_top], dir->entries[0].name, MAX_NAME_LENGTH);
        stack_top++;

        // Move to parent directory
        dir_index = dir->parent_index;
    }

    // Build the final path from stack
    strcat(temp_path, "/");
    for (int i = stack_top - 1; i >= 0; i--) {
        strcat(temp_path, stack[i]);
        if (i > 0) strcat(temp_path, "/"); // Add slashes between directories
    }

    // Copy to output
    strncpy(output_path, temp_path, MAX_PATH_LENGTH);
}


// change cwd in 2 cases: absolute / relative path was given
int chdir(const char *path) {
    char new_path[MAX_PATH_LENGTH];

    if(!find_directory_index(path)) {
        // not a valid path in the tree
        return -1;
    }
    else {
        if (path[0] == '/'){
            strncpy(current_working_directory, path, MAX_PATH_LENGTH);
        }
        else{
            
            char *path_copy = strdup(new_path);
            char *token = strtok(path_copy, "/");

            int current_dir_index = (path[0] == '/') ? 0 : cwd_index;

            while (token) {
                if (strcmp(token, "..") == 0) {
                    if (current_dir_index != 0) {
                        current_dir_index = fs_metadata.directories[current_dir_index].parent_index;
                    }
                } else if (strcmp(token, ".") != 0) {
                    // Move into the directory if valid
                    // I might need a loop actually
                    int next_index = find_directory_index(token);
                    if (next_index == -1) {
                        printf("Error: Directory '%s' not found.\n", token);
                        free(path_copy);
                        return -1;
                    }
                    current_dir_index = next_index;
                }
        
                token = strtok(NULL, "/");
            }
            free(path_copy);
            char *final_path;
            get_full_path_from_index(current_dir_index, final_path);
            strncpy(current_working_directory, final_path, MAX_PATH_LENGTH);
            cwd_index = current_dir_index;
        
            return 0;
        }
    }
}