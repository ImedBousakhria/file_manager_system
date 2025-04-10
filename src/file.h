#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"


int create_file(const char *filename, char* parent_path, int user_index, int permissions);
void get_full_path_from_index(int dir_index, char *output_path);
void read_from_file(const char *parent_path, const char *filename, char *buffer, int buffer_size, int user_index);
void seek_to_location(FILE *disk_file, int block_idx, int offset_in_block);
void write_to_file(const char *parent_path, const char *filename, const char *data, int user_index);
char* read_full_file(const char *parent_path, const char *filename, int user_index);
void delete_inode(int inode_index, int user_index);
void move_file(const char* path, const char* des_path, int user_index);

#endif
