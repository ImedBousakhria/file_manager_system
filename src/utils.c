#include <stdio.h>
#include <stdlib.h>
#include "fs.h"


int find_free_inode(){
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

// Function to find the directory index corresponding to a path
int find_directory_index(const char *path){
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

