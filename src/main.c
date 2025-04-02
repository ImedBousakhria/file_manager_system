#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
#include "file.h"
#include "fs.h"
#include "permissions.h"
#include "utils.h"

int main () {
    printf("Hello m1");
    disk_init();
    printf("Hello main2");
    // mount_fs();
    

    return EXIT_SUCCESS;
}