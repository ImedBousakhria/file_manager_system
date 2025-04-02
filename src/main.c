#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
#include "file.h"
#include "fs.h"
#include "permissions.h"
#include "utils.h"

int main () {
    disk_init();
    // mount_fs();

    // int file_inode1 = create_file("imed_test6.txt", "/", 1);
    // printf("%d", file_inode1);
    write_to_file("/", "imed_test6.txt", "Hello world");
    char buf[20] = {0};
    read_from_file("/", "imed_test6.txt", buf, 20);
    buf[19] = '\0';
    printf("%s", buf);
    return 0;
}