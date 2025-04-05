#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include "disk.h"
#include "file.h"
#include "fs.h"
#include "permissions.h"
#include "utils.h"
#include "folder.h"

// Function to display help message
void display_help() {
    printf("File System Command Line Interface\n");
    printf("Usage: ./main [OPTIONS] [ARGUMENTS]\n\n");
    printf("Options:\n");
    printf("  -h                                Display this help message\n");
    printf("  -f filename path user perm        Create a file with given name, path, user index, and permissions\n");
    printf("  -w filename path user content     Write content to a file as specified user\n");
    printf("  -r filename path size             Read from a file (specify size in bytes)\n");
    printf("  -d dirname path user perm         Create a directory with given name, path, user index, and permissions\n");
    printf("  -c path                          Change current working directory\n");
    printf("  -p                               Print current working directory\n");
    printf("  -l                               List all files and directories from root\n");
    printf("  -df filename path                Delete a file\n");
    printf("  -dd path                         Delete a directory\n");
    printf("  -cf filename path user perm      Change file permissions (chmod)\n");
    printf("  -i                               Print inode information\n");
    printf("  -di                              Print directory information\n");
}

int main(int argc, char *argv[]) {
    // Initialize the file system
    disk_init();
    printf("hehehe");
    move_file("/feryel/dir/text.text");
    // If no arguments are provided, show help
    if (argc < 2) {
        display_help();
        return 0;
    }
    
    // Parse command-line arguments
    if (strcmp(argv[1], "-h") == 0) {
        display_help();
    }
    // Create a file: -f filename path user_index permissions
    else if (strcmp(argv[1], "-f") == 0) {
        if (argc < 6) {
            printf("Error: Not enough arguments for file creation.\n");
            return 1;
        }
        char *filename = argv[2];
        char *path = argv[3];
        int user_index = atoi(argv[4]);
        int permissions = strtol(argv[5], NULL, 8); // Octal permissions
        
        int result = create_file(filename, path, user_index, permissions);
        if (result >= 0) {
            printf("File created successfully with inode %d\n", result);
        }
    }
    // Write to a file: -w filename path user_index content
    else if (strcmp(argv[1], "-w") == 0) {
        if (argc < 6) {
            printf("Error: Not enough arguments for writing to file.\n");
            return 1;
        }
        char *filename = argv[2];
        char *path = argv[3];
        int user_index = atoi(argv[4]);
        char *content = argv[5];
        
        write_to_file(path, filename, content, user_index);
    }
    // Read from a file: -r filename path size
    else if (strcmp(argv[1], "-r") == 0) {
        if (argc < 5) {
            printf("Error: Not enough arguments for reading from file.\n");
            return 1;
        }
        char *filename = argv[2];
        char *path = argv[3];
        int size = atoi(argv[4]);
        
        char *buffer = (char *)malloc(size + 1);
        if (!buffer) {
            printf("Error: Memory allocation failed.\n");
            return 1;
        }
        memset(buffer, 0, size + 1);
        
        read_from_file(path, filename, buffer, size);
        printf("Content read from file:\n%s\n", buffer);
        free(buffer);
    }
    // Create a directory: -d dirname path user_index permissions
    else if (strcmp(argv[1], "-d") == 0) {
        if (argc < 6) {
            printf("Error: Not enough arguments for directory creation.\n");
            return 1;
        }
        char *dirname = argv[2];
        char *path = argv[3];
        int user_index = atoi(argv[4]);
        int permissions = strtol(argv[5], NULL, 8); // Octal permissions
        
        create_directory(dirname, path, user_index, permissions);
    }
    // Change directory: -c path
    else if (strcmp(argv[1], "-c") == 0) {
        if (argc < 3) {
            printf("Error: No path specified for changing directory.\n");
            return 1;
        }
        char *path = argv[2];
        
        if (chdir(path) == 0) {
            printf("Changed directory to: %s\n", getcwd());
        } else {
            printf("Error changing directory to: %s\n", path);
        }
    }
    // Print current working directory: -p
    else if (strcmp(argv[1], "-p") == 0) {
        printf("Current working directory: %s\n", getcwd());
    }
    // List all from root: -l
    else if (strcmp(argv[1], "-l") == 0) {
        list_all_from_root();
    }
    // Delete a file: -df filename path
    else if (strcmp(argv[1], "-df") == 0) {
        if (argc < 4) {
            printf("Error: Not enough arguments for file deletion.\n");
            return 1;
        }
        char *filename = argv[2];
        char *path = argv[3];
        
        // Find the file's inode index
        int dir_idx = find_directory_index(path);
        if (dir_idx == -1) {
            printf("Error: Directory path not found.\n");
            return 1;
        }
        
        Directory *parent_directory = &fs_metadata.directories[dir_idx];
        int file_inode_idx = -1;
        
        for (int i = 0; i < parent_directory->num_entries; i++) {
            if (strcmp(parent_directory->entries[i].name, filename) == 0 && 
                parent_directory->entries[i].isfile == 1) {
                file_inode_idx = parent_directory->entries[i].inode_index;
                break;
            }
        }
        
        if (file_inode_idx == -1) {
            printf("Error: File not found.\n");
            return 1;
        }
        
        delete_inode(file_inode_idx);
        printf("File '%s' deleted successfully.\n", filename);
    }
    // Delete a directory: -dd path
    else if (strcmp(argv[1], "-dd") == 0) {
        if (argc < 3) {
            printf("Error: No path specified for directory deletion.\n");
            return 1;
        }
        char *path = argv[2];
        
        delete_dir(path);
    }
    // Change file permissions: -cf filename path user_index permissions
    else if (strcmp(argv[1], "-cf") == 0) {
        if (argc < 6) {
            printf("Error: Not enough arguments for changing file permissions.\n");
            return 1;
        }
        char *filename = argv[2];
        char *path = argv[3];
        char *username = fs_metadata.users[atoi(argv[4])].name;
        int permissions = strtol(argv[5], NULL, 8); // Octal permissions
        
        // Find the file's inode index
        int dir_idx = find_directory_index(path);
        if (dir_idx == -1) {
            printf("Error: Directory path not found.\n");
            return 1;
        }
        
        Directory *parent_directory = &fs_metadata.directories[dir_idx];
        int file_inode_idx = -1;
        
        for (int i = 0; i < parent_directory->num_entries; i++) {
            if (strcmp(parent_directory->entries[i].name, filename) == 0 && 
                parent_directory->entries[i].isfile == 1) {
                file_inode_idx = parent_directory->entries[i].inode_index;
                break;
            }
        }
        
        if (file_inode_idx == -1) {
            printf("Error: File not found.\n");
            return 1;
        }
        
        chmod_file(username, file_inode_idx, permissions);
    }
    // Print all inodes: -i
    else if (strcmp(argv[1], "-i") == 0) {
        print_all_inodes();
    }
    // Print all directories: -di
    else if (strcmp(argv[1], "-di") == 0) {
        print_all_dir();
    }
    else {
        printf("Error: Unknown option '%s'.\n", argv[1]);
        display_help();
        return 1;
    }
    
    return 0;
}