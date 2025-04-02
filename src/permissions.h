#ifndef PERMISSION_H
#define PERMISSION_H

#include "fs.h"

// Definition of permissions in octal
// To facilitate checks with binary masks
#define PERM_READ    4  // binary 100
#define PERM_WRITE   2  // binary 010
#define PERM_EXECUTE 1  // binary 001

// Definition of groups
#define GROUP_ADMIN  0
#define GROUP_USER   1
#define NUM_GROUPS   2


// Permission verification functions
int check_permission(const char* username, int inode_idx, int permission_type);
int user_can_read(const char* username, int inode_idx);
int user_can_write(const char* username, int inode_idx);
int user_can_execute(const char* username, int inode_idx);
int is_owner(const char* username, int inode_idx);
int is_in_group(const char* username, int group_id);

// Permission modification functions
int chmod_file(const char* username, int inode_idx, int new_permissions);

// Helper function to display permissions
void print_permissions(int permissions);

#endif /* PERMISSION_H */