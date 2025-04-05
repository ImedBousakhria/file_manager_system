#include <stdio.h>
#include <stdlib.h>
#include "fs.h"



int find_free_inode(){
    int nb_inodes = fs_metadata.nb_inodes;
    printf("nb of inodes  %d", nb_inodes);
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

/**
 * adding an entry to a directory
 * type if file then 1
 * returns the it's index in the entries table -1 if failed 
 */
int add_entry(int dir_index, int entry_index, int type, const char * name){
    DirectoryEntry entry;
    entry.inode_index = entry_index;
    entry.isfile = type;
    strcpy(entry.name, name);
    Directory *dis_dir = &fs_metadata.directories[dir_index];
    int num_entries = dis_dir->num_entries;
    if(num_entries >= MAX_ENTRIES_PER_DIR){
        return -1;
    }
    dis_dir->entries[num_entries] = entry;
    dis_dir->num_entries = num_entries +1;
    return num_entries;

}