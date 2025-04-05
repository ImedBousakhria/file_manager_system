#ifndef DIR_H
#define DIR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

void create_directory(const char *dirname, const char* parent_path, int user_index, int permission);
int chdir(const char *path);
const char* getcwd();
void delete_dir(const char *path);
void move_directory(const char* path, const char* des_path);

#endif
