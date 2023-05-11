CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -pthread
LDFLAGS =
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
BIN_FILE = $(BIN_DIR)/multithreadedfiletransfer.out

.PHONY: all clean

all: $(BIN_FILE)

$(BIN_FILE): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

link: $(OBJ_FILES)
	$(CC) $(LDFLAGS) $^ -o $(BIN_FILE)

clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_FILE)
