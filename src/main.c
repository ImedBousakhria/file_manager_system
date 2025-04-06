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
    
    int file_inode1 = create_file("imed.txt", "/", 1, 700);
    printf("%d", file_inode1);
  //  int modify=chmod_file(fs_metadata.users[1].name,file_inode1,777);
   
    write_to_file("/", "imed.txt", "23hHello wworl", 0);
    unsigned char *buf;
    read_from_file("/", "imed.txt", buf, 20);
    printf("%s", buf);
    char *content = read_full_file("/", "imed.txt");
    if (content) {
      printf("File content:\n%s\n", content);
      free(content); 
}
    // printf("%d", strlen(buf));
    
    // create_directory("dir3", "/dir1/dir2", 1, 777);
    // create_directory("dir1", "/", 1, 777);
    //  delete_dir();
    // printf("%s\n", getcwd());
    // // create_directory("dir2", "/dir1", 1, 777);
    // chdir("/dir1/dir2/dir3");
    // printf("%s\n", getcwd());
    // chdir("../");
    // printf("%s\n", getcwd());
    // delete_dir("/dir1/dir2");
    // delete_inode(1);
    // list_all_from_root();
    // print_all_inodes();
    // print_all_dir();
    return 0;
}