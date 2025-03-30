#include <stdio.h>
#include <stdlib.h>
#include "fs.h"


void create_file(const char *filename, int parent_inode_idx) {
    Inode *inode = &fs.inodes[parent_inode_idx];

    if (!inode->isDirectory) {
        printf("Error: Parent must be a directory.\n");
        return;
    }

    // Find free inode
    int new_inode_idx = find_free_inode();
    if (new_inode_idx == -1) {
        printf("Error: No free inodes.\n");
        return;
    }

    // Initialize file inode
    fs.inodes[new_inode_idx].isDirectory = 0;
    fs.inodes[new_inode_idx].used = 1;
    fs.inodes[new_inode_idx].size = 0;
    fs.inodes[new_inode_idx].permissions = 777;
    fs.inodes[new_inode_idx].blocks = malloc(sizeof(int) * 1);  // Start with 1 block

    // Add to parent directory
    inode->directory.num_entries++;
    inode->directory.entries = realloc(inode->directory.entries, 
        inode->directory.num_entries * sizeof(DirectoryEntry));
    
    DirectoryEntry *entry = &inode->directory.entries[inode->directory.num_entries - 1];
    strcpy(entry->name, filename);
    entry->inode_index = new_inode_idx;
}
