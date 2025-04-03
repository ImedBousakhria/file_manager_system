# Makefile for File System Project

# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Create directories if they don't exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# Source files and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Target executable
TARGET = $(BIN_DIR)/file_manager

# Main rule
all: $(TARGET)

# Link rule
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile rule
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET) $(SRC_DIR)/*.exe fs_vdisk.img

# Run rule
run: $(TARGET)
	$(TARGET)

.PHONY: all clean run