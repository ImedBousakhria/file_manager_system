#ifndef PERMISSION_H
#define PERMISSION_H

#include "fs.h"

// Définition des permissions en octal
// Pour faciliter les vérifications avec des masques binaires
#define PERM_READ    4  // 100 binaire
#define PERM_WRITE   2  // 010 binaire
#define PERM_EXECUTE 1  // 001 binaire

// Définition des groupes
#define GROUP_ADMIN  0
#define GROUP_USER   1
#define NUM_GROUPS   2


// Fonctions de vérification des permissions
int check_permission(const char* username, int inode_idx, int permission_type);
int user_can_read(const char* username, int inode_idx);
int user_can_write(const char* username, int inode_idx);
int user_can_execute(const char* username, int inode_idx);
int is_owner(const char* username, int inode_idx);
int is_in_group(const char* username, int group_id);

// Fonctions de modification des permissions
int chmod_file(const char* username, int inode_idx, int new_permissions);

// Fonction auxiliaire pour afficher les permissions
void print_permissions(int permissions);

#endif /* PERMISSION_H */