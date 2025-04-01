#include <stdio.h>
#include <stdlib.h>
#include "fs.h"

int main (int argc, char *argv[]) {
    return EXIT_SUCCESS;
}

void create_directory(const char *dirname, int parent_inode_idx) {
    Inode *inode = &fs_metadata.inodes[parent_inode_idx];

    if (!inode->isDirectory) {
        printf("Error: Parent must be a directory.\n");
        return;
    }

    int new_inode_idx = find_free_inode();
    if (new_inode_idx == -1) {
        printf("Error: No free inodes.\n");
        return;
    }

    // Initialize directory inode
    fs_metadata.inodes[new_inode_idx].isDirectory = 1;
    fs_metadata.inodes[new_inode_idx].used = 1;
    fs_metadata.inodes[new_inode_idx].size = 0;
    fs_metadata.inodes[new_inode_idx].permissions = 777;
    fs_metadata.inodes[new_inode_idx].directory.num_entries = 2;  // . and ..

    // Allocate initial space for 2 entries
    fs_metadata.inodes[new_inode_idx].directory.entries = malloc(sizeof(DirectoryEntry) * 2);
    
    // "." (self)
    strcpy(fs_metadata.inodes[new_inode_idx].directory.entries[0].name, ".");
    fs_metadata.inodes[new_inode_idx].directory.entries[0].inode_index = new_inode_idx;
    
    // ".." (parent)
    strcpy(fs_metadata.inodes[new_inode_idx].directory.entries[1].name, "..");
    fs_metadata.inodes[new_inode_idx].directory.entries[1].inode_index = parent_inode_idx;

    // Add new dir to parent
    inode->directory.num_entries++;
    inode->directory.entries = realloc(inode->directory.entries, 
        inode->directory.num_entries * sizeof(DirectoryEntry));
    
    DirectoryEntry *entry = &inode->directory.entries[inode->directory.num_entries - 1];
    strcpy(entry->name, dirname);
    entry->inode_index = new_inode_idx;

    // we should save the fs_metadata 
    
}
