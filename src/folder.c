#include <stdio.h>
#include <stdlib.h>
#include "fs.h"
#include "utils.h"



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
    strcpy(self_entry.name, dirname);

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

/**
 * deleting a directory
 * here we get the last dir if it exist if not just make the num_directories-1 we keep the root
 * we recursivelly delete everything under it 
 */
void delete_dir(const char *path){

    int dir_index = find_directory_index(path);
    if(dir_index < 0){
        printf("Error: path doesn't exist");
        return;
    }
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

        if( i == 0 || i == 1) {
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


const char* getcwd() {
    return current_working_directory;
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