# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lpq -lssl -lcrypto

# Directory structure
SRC_DIR = src
UTILS_DIR = utils
BUILD_DIR = build
INCLUDE_DIR = include

# Source and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(UTILS_DIR)/*.c)
OBJ_FILES = $(patsubst %.c, $(BUILD_DIR)/%.o, $(notdir $(SRC_FILES)))

# Target executable
TARGET = $(BUILD_DIR)/bank_project

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJ_FILES) -o $@ $(LDFLAGS)

# Compile source files from src directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile source files from utils directory
$(BUILD_DIR)/%.o: $(UTILS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if it doesn't exist
$(BUILD_DIR):
	@mkdir -p $@

# Clean up build directory
clean:
	rm -rf $(BUILD_DIR)

# Declare phony targets
.PHONY: all clean