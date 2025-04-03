#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "file.h"
#include "fs.h"
#include "permissions.h"
#include "utils.h"
#include "folder.h"

int main () {
    disk_init();
    // mount_fs();

    // int file_inode1 = create_file("imed_test6.txt", "/", 1);
    // printf("%d", file_inode1);
    // write_to_file("/", "imed_test6.txt", "Hello world");
    // char buf[13];
    // read_from_file("/", "imed_test6.txt", buf, 20);
    // printf("%s", buf);
    // printf("%d", strlen(buf));
    
    // create_directory("dir1", "/", 1, 777);
    //  delete_dir();
    // create_directory("dir2", "/dir1", 1, 777);
    chdir("/dir1/dir2");
    printf("%s\n", getcwd());
    chdir("");
    printf("%s", getcwd());
    list_all_from_root();
    return 0;
}