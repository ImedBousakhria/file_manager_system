#include <stdio.h>
#include <stdlib.h>
#include "permissions.h"
#include "fs.h"


// User initialization
// This function is called during filesystem initialization
void init_users() {
    // Initialize the 3 predefined users
    
    // Dyhia - admin (group 0)
    User* dyhia = &fs_metadata.users[0];
    dyhia->name = "dyhia";
    dyhia->groupe = GROUP_ADMIN;
    
    // Feryel - admin (group 0)
    User* feryel = &fs_metadata.users[1];
    feryel->name = "feryel";
    feryel->groupe = GROUP_ADMIN;
    
    // Imad - standard user (group 1)
    User* imad = &fs_metadata.users[2];
    imad->name = "imad";
    imad->groupe = GROUP_USER;
}



// Find a user by their name
int find_user_by_name(const char* username) {
    for (int i = 0; i < 3; i++) {
        if (strcmp(fs_metadata.users[i].name, username) == 0) {
            return i;
        }
    }
    return -1;  // User not found
}

// Check if the user is in a specific group
int is_in_group(const char* username, int group_id) {
    int user_idx = find_user_by_name(username);
    if (user_idx == -1) {
        return 0;  // User not found
    }
    
    return fs_metadata.users[user_idx].groupe == group_id;
}

// Check if the user is the owner of an inode
int is_owner(const char* username, int inode_idx) {
    if (inode_idx < 0 || inode_idx >= fs_metadata.nb_inodes || !fs_metadata.inodes[inode_idx].used) {
        return 0;  // Invalid inode
    }
    
    int user_idx = find_user_by_name(username);
    if (user_idx == -1) {
        return 0;  // User not found
    }
    
    return fs_metadata.inodes[inode_idx].owner_indx == user_idx;
}

// Check if a user has a specific permission on an inode

// permission type: for example I have defined 3 variables in permissions.h, if I want to check read permission I need to type in 4 
int check_permission(const char* username, int inode_idx, int permission_type) {
    // Check if the inode exists and is in use
    if (inode_idx < 0 || inode_idx >= fs_metadata.nb_inodes || !fs_metadata.inodes[inode_idx].used) {
        printf("Error: Invalid or unused inode.\n");
        return 0;
    }
    
    // Find the user
    int user_idx = find_user_by_name(username);
    if (user_idx == -1) {
        printf("Error: User '%s' not found.\n", username);
        return 0;
    }
    
    // Administrators (group 0) have all permissions
    if (fs_metadata.users[user_idx].groupe == GROUP_ADMIN) {
        return 1;
    }
    
    // Get the inode permissions
    int perms = fs_metadata.inodes[inode_idx].permissions;
    
    // Check if the user is the owner
    if (fs_metadata.inodes[inode_idx].owner_indx == user_idx) {
        // Check owner permissions (bits 6-8)
        return ((perms >> 6) & 0x7 & permission_type) == permission_type;
    }
    
    // Check if the user is in the same group as the owner

    if (fs_metadata.users[user_idx].groupe == GROUP_USER) {
        // Check group permissions (bits 3-5)
        return ((perms >> 3) & 0x7 & permission_type) == permission_type;
    }
    
    // Otherwise, check permissions for others (bits 0-2)
    return (perms & 0x7 & permission_type) == permission_type;
}

// Check if a user can read a file
int user_can_read(const char* username, int inode_idx) {
    return check_permission(username, inode_idx, PERM_READ);
}

// Check if a user can write to a file
int user_can_write(const char* username, int inode_idx) {
    return check_permission(username, inode_idx, PERM_WRITE);
}

// Check if a user can execute a file
int user_can_execute(const char* username, int inode_idx) {
    return check_permission(username, inode_idx, PERM_EXECUTE);
}


// Function to modify file permissions (equivalent to chmod)
int chmod_file(const char* username, int inode_idx, int new_permissions) {
    // Check if the inode exists and is in use
    if (inode_idx < 0 || inode_idx >= fs_metadata.nb_inodes || !fs_metadata.inodes[inode_idx].used) {
        printf("Error: Invalid or unused inode.\n");
        return 0;
    }
    
    // Find the user
    int user_idx = find_user_by_name(username);
    if (user_idx == -1) {
        printf("Error: User '%s' not found.\n", username);
        return 0;
    }
    
    // Only the owner or an administrator can modify permissions
    if (fs_metadata.users[user_idx].groupe == GROUP_ADMIN ||  fs_metadata.inodes[inode_idx].owner_indx == user_idx) {
        
        // Check that the permissions are valid (between 0 and 0777)
        if (new_permissions < 0 || new_permissions > 0777) {
            printf("Error: Invalid permission value.\n");
            return 0;
        }
        
        fs_metadata.inodes[inode_idx].permissions = new_permissions;
        printf("Permissions for inode %d modified to ", inode_idx);
        print_permissions(new_permissions);
        return 1;
    } else {
        printf("Error: Permission denied. Only the owner or an administrator can modify permissions.\n");
        return 0;
    }
}

// Helper function to display permissions in readable format
void print_permissions(int permissions) {
    // Convert octal permissions to rwx format
    char perm_str[10] = "---------";
    
    // Owner permissions (bits 6-8)
    if (permissions & (PERM_READ << 6)) perm_str[0] = 'r';
    if (permissions & (PERM_WRITE << 6)) perm_str[1] = 'w';
    if (permissions & (PERM_EXECUTE << 6)) perm_str[2] = 'x';
    
    // Group permissions (bits 3-5)
    if (permissions & (PERM_READ << 3)) perm_str[3] = 'r';
    if (permissions & (PERM_WRITE << 3)) perm_str[4] = 'w';
    if (permissions & (PERM_EXECUTE << 3)) perm_str[5] = 'x';
    
    // Others permissions (bits 0-2)
    if (permissions & PERM_READ) perm_str[6] = 'r';
    if (permissions & PERM_WRITE) perm_str[7] = 'w';
    if (permissions & PERM_EXECUTE) perm_str[8] = 'x';
    
    printf("%s (0%o)\n", perm_str, permissions);
}