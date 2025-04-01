#ifndef PERMISSION_H
#define PERMISSION_H

#include "fs.h"

// Définition des permissions en octal
// Pour faciliter les vérifications avec des masques binaires
#define PERM_READ    4  // 100 binaire
#define PERM_WRITE   2  // 010 binaire
#define PERM_EXECUTE 1  // 001 binaire

