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

        // unique loop, because we're jump searching (not sequencial)
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
    if(new_inode_idx == n_inodes){
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
