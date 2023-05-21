# Compiler and flags
CC := gcc 
CFLAGS := -Wall -Wextra -g -pthread

# Directories
BIN_DIR := bin
OBJ_DIR := obj
CLIENT_DIR := src/client
SERVER_DIR := src/server

# Targets
CLIENT_TARGET := $(BIN_DIR)/multithreadedfiletransfer.out
SERVER_TARGET := $(BIN_DIR)/server.out

# Source files
CLIENT_SRCS := $(wildcard $(CLIENT_DIR)/*.c)
SERVER_SRCS := $(wildcard $(SERVER_DIR)/*.c)

# Object files
CLIENT_OBJS := $(patsubst $(CLIENT_DIR)/%.c,$(OBJ_DIR)/client/%.o,$(CLIENT_SRCS))
SERVER_OBJS := $(patsubst $(SERVER_DIR)/%.c,$(OBJ_DIR)/server/%.o,$(SERVER_SRCS))

# Phony targets
.PHONY: all clean

# Default target
all: $(CLIENT_TARGET) $(SERVER_TARGET)

# Rule to compile the client executable
$(CLIENT_TARGET): $(CLIENT_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to compile the server executable
$(SERVER_TARGET): $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to compile client object files
$(OBJ_DIR)/client/%.o: $(CLIENT_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/client
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile server object files
$(OBJ_DIR)/server/%.o: $(SERVER_DIR)/%.c
	@mkdir -p $(OBJ_DIR)/server
	$(CC) $(CFLAGS) -c $< -o $@

# Clean the build artifacts
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
