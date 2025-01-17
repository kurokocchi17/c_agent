# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -I./include

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
LIB_DIR = lib

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Output library name
LIB = $(LIB_DIR)/libai_dancer.a

# Dependencies
LIBS = -lwebsockets -ljson-c -lcurl

# Make sure the directories exist
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR))

# Default target
all: $(LIB)

# Create static library
$(LIB): $(OBJS)
	ar rcs $@ $^

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build examples
examples: $(LIB)
	$(CC) $(CFLAGS) examples/basic_chat.c -o $(BIN_DIR)/basic_chat $(LIB) $(LIBS)

# Clean build files
clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

# Install headers and library
install: $(LIB)
	mkdir -p /usr/local/include/ai_dancer
	mkdir -p /usr/local/lib
	cp $(INC_DIR)/*.h /usr/local/include/ai_dancer/
	cp $(LIB) /usr/local/lib/

# Uninstall
uninstall:
	rm -rf /usr/local/include/ai_dancer
	rm -f /usr/local/lib/libai_dancer.a

.PHONY: all clean install uninstall examples 