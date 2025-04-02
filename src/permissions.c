#include <stdio.h>
#include <stdlib.h>
#include "permissions.h"
#include "fs.h"


// Initialisation des utilisateurs
// Cette fonction est appelée lors de l'initialisation du système de fichiers
void init_users() {
    // Initialiser les 3 utilisateurs prédéfinis
    
    // Dyhia - admin (groupe 0)
    User* dyhia = &fs_metadata.users[0];
    dyhia->name = "dyhia";
    dyhia->groupe = GROUP_ADMIN;
    
    // Feryel - admin (groupe 0)
    User* feryel = &fs_metadata.users[1];
    feryel->name = "feryel";
    feryel->groupe = GROUP_ADMIN;
    
    // Imad - utilisateur standard (groupe 1)
    User* imad = &fs_metadata.users[2];
    imad->name = "imad";
    imad->groupe = GROUP_USER;
}



// Trouver un utilisateur par son nom
int find_user_by_name(const char* username) {
    for (int i = 0; i < 3; i++) {
        if (strcmp(fs_metadata.users[i].name, username) == 0) {
            return i;
        }
    }
    return -1;  // Utilisateur non trouvé
}

// Vérifier si l'utilisateur est dans un groupe spécifique
int is_in_group(const char* username, int group_id) {
    int user_idx = find_user_by_name(username);
    if (user_idx == -1) {
        return 0;  // Utilisateur non trouvé
    }
    
    return fs_metadata.users[user_idx].groupe == group_id;
}

// Vérifier si l'utilisateur est le propriétaire d'un inode
int is_owner(const char* username, int inode_idx) {
    if (inode_idx < 0 || inode_idx >= fs_metadata.nb_inodes || !fs_metadata.inodes[inode_idx].used) {
        return 0;  // Inode invalide
    }
    
    int user_idx = find_user_by_name(username);
    if (user_idx == -1) {
        return 0;  // Utilisateur non trouvé
    }
    
    return fs_metadata.inodes[inode_idx].owner_indx == user_idx;
}

// Vérifier si un utilisateur a une permission spécifique sur un inode

// permission type : for exemple i have defined 3 variables in permissions.h , if i want to check permission read i need to type in 4 
int check_permission(const char* username, int inode_idx, int permission_type) {
    // Vérifier si l'inode existe et est utilisé
    if (inode_idx < 0 || inode_idx >= fs_metadata.nb_inodes || !fs_metadata.inodes[inode_idx].used) {
        printf("Erreur: Inode invalide ou inutilisé.\n");
        return 0;
    }
    
    // Trouver l'utilisateur
    int user_idx = find_user_by_name(username);
    if (user_idx == -1) {
        printf("Erreur: Utilisateur '%s' non trouvé.\n", username);
        return 0;
    }
    
    // Les administrateurs (groupe 0) ont toutes les permissions
    if (fs_metadata.users[user_idx].groupe == GROUP_ADMIN) {
        return 1;
    }
    
    // Récupérer les permissions de l'inode
    int perms = fs_metadata.inodes[inode_idx].permissions;
    
    // Vérifier si l'utilisateur est le propriétaire
    if (fs_metadata.inodes[inode_idx].owner_indx == user_idx) {
        // Vérifier les permissions du propriétaire (bits 6-8)
        return ((perms >> 6) & 0x7 & permission_type) == permission_type;
    }
    
    // Vérifier si l'utilisateur est dans le même groupe que le propriétaire

    if (fs_metadata.users[user_idx].groupe == GROUP_USER) {
        // Vérifier les permissions de groupe (bits 3-5)
        return ((perms >> 3) & 0x7 & permission_type) == permission_type;
    }
    
    // Sinon, vérifier les permissions pour les autres (bits 0-2)
    return (perms & 0x7 & permission_type) == permission_type;
}

// Vérifier si un utilisateur peut lire un fichier
int user_can_read(const char* username, int inode_idx) {
    return check_permission(username, inode_idx, PERM_READ);
}

// Vérifier si un utilisateur peut écrire dans un fichier
int user_can_write(const char* username, int inode_idx) {
    return check_permission(username, inode_idx, PERM_WRITE);
}

// Vérifier si un utilisateur peut exécuter un fichier
int user_can_execute(const char* username, int inode_idx) {
    return check_permission(username, inode_idx, PERM_EXECUTE);
}


// Fonction pour modifier les permissions d'un fichier (équivalent de chmod)
int chmod_file(const char* username, int inode_idx, int new_permissions) {
    // Vérifier si l'inode existe et est utilisé
    if (inode_idx < 0 || inode_idx >= fs_metadata.nb_inodes || !fs_metadata.inodes[inode_idx].used) {
        printf("Erreur: Inode invalide ou inutilisé.\n");
        return 0;
    }
    
    // Trouver l'utilisateur
    int user_idx = find_user_by_name(username);
    if (user_idx == -1) {
        printf("Erreur: Utilisateur '%s' non trouvé.\n", username);
        return 0;
    }
    
    // Seul le propriétaire ou un administrateur peut modifier les permissions
    if (fs_metadata.users[user_idx].groupe == GROUP_ADMIN ||  fs_metadata.inodes[inode_idx].owner_indx == user_idx) {
        
        // Vérifier que les permissions sont valides (entre 0 et 0777)
        if (new_permissions < 0 || new_permissions > 0777) {
            printf("Erreur: Valeur de permission invalide.\n");
            return 0;
        }
        
        fs_metadata.inodes[inode_idx].permissions = new_permissions;
        printf("Permissions de l'inode %d modifiées à ", inode_idx);
        print_permissions(new_permissions);
        return 1;
    } else {
        printf("Erreur: Permission refusée. Seul le propriétaire ou un administrateur peut modifier les permissions.\n");
        return 0;
    }
}

// Fonction auxiliaire pour afficher les permissions en format lisible
void print_permissions(int permissions) {
    // Convertir permissions octales en format rwx
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