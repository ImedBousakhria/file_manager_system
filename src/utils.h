#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "disk.h"


int entry_exists(const char *name, int dir_index, int isfile);
int find_index_entry(int index, int parent_index, int type);
void delete_entry(int inode_index, int parent_index, int type);
int find_directory_index(const char *path);
int find_free_inode();
void get_full_path_from_index(int dir_index, char *output_path);

#endif
