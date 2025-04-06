# Mini Filesystem

A custom UNIX-like file system implemented in C using inode-based fragmentation, providing comprehensive file and directory management capabilities.

## 🔗 Repository
[https://github.com/ImedBousakhria/file_manager_system](https://github.com/ImedBousakhria/file_manager_system)

## 🌟 Key Features

✅ **Inode-Based Fragmentation**
  - Advanced storage management using inode system
  - Efficient file and directory tracking

✅**Comprehensive File Operations**
  - Create, read, write, and delete files
  - Create and delete directories
  - Change directory and list contents
  - Manage file permissions

✅ **Multi-User Support**
  - User-based permission system
  - Supports multiple user indexes
  - Fine-grained access control

✅ **Command-Line Interface**
  - Intuitive CLI for filesystem interactions
  - Multiple command options for various operations

## 🛠 Features in Detail

### Supported Operations
- File Creation: `-f filename path user_index permissions`
- File Writing: `-w filename path user_index content`
- File Reading: `-r filename path user_index size`
- Directory Creation: `-d dirname path user_index permissions`
- Change Directory: `-c path`
- Print Current Directory: `-p`
- List Contents: `-l`
- Delete File: `-df filename path`
- Delete Directory: `-dd path`
- Change File Permissions: `-cf filename path user_index permissions`

### Permission System
- Octal-based permission model (similar to UNIX)
- User-specific access control
- Read, Write, Execute permissions

## 🤝 Contributors

- [Imed Bousakhria](https://github.com/ImedBousakhria)
- [Dyhia Boudjema](https://github.com/BoudjemaDyhia)
- [Feryel Boubeker](https://github.com/FeryelBoubeker)

## 🛠 Prerequisites

- GCC Compiler
- Make
- Basic understanding of C programming
- UNIX-like operating system (Linux/macOS recommended)

## 🚀 Installation & Compilation

1. Clone the repository:
   ```bash
   git clone https://github.com/ImedBousakhria/file_manager_system.git
   cd file_manager_system
   ```

2. Compile the project:
   ```bash
   make
   ```

## 📖 Usage Examples

### Create a File
```bash
./main -f myfile.txt /home 0 644
```

### Write to a File
```bash
./main -w myfile.txt /home 0 "Hello, Mini Filesystem!"
```

### Read a File
```bash
./main -r myfile.txt /home 0 100
```

### Create a Directory
```bash
./main -d documents /home 0 755
```

## 📝 Project Structure

```
file_manager_system/
│
├── src/
│   ├── disk.c          # Disk management
│   ├── file.c          # File operations
│   ├── folder.c        # Directory operations
│   ├── fs.c            # Filesystem core
│   ├── main.c          # CLI interface
│   ├── permissions.c   # Permission handling
│   └── utils.c         # Utility functions
│
├── include/            # Header files
│   ├── disk.h
│   ├── file.h
│   └── ... (other headers)
│
├── Makefile
└── README.md
```

## 🔧 Key Components

- **Inode Management**: Custom inode system for tracking file metadata
- **User Permissions**: Multi-user access control
- **Fragmentation Handling**: Efficient storage allocation
- **CLI Interface**: Easy-to-use command-line interactions

## 🚧 Limitations

- Single-file filesystem implementation
- Limited to preset number of users and directories
- Simplified permission model

## 📞 Support

For issues or questions, please open an issue in the GitHub repository.

---

**Happy Filesystem Exploring! 🗂️**









