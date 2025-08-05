CC = gcc
CFLAGS = -Wall -Iinclude -Wextra -pedantic -std=c11
LDFLAGS = -lm

SRC_DIR = src
TEST_DIR = test
INCLUDE_DIR = include
BUILD_DIR = build
SCRIPTS_DIR = scripts
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)

MAIN_SRC = $(SRC_DIR)/main.c
SRC_FILES_NO_MAIN = $(filter-out $(MAIN_SRC), $(SRC_FILES))

OBJS = $(SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
OBJS_NO_MAIN = $(SRC_FILES_NO_MAIN:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TEST_BINS = $(TEST_SRCS:$(TEST_DIR)/%.c=$(BIN_DIR)/%)
MEM_TEST = $(SCRIPTS_DIR)/mem_test.sh 

TARGET = eval

all: $(TARGET)

# Compile .c to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Final executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Compile test binaries without main.o
$(BIN_DIR)/%: $(TEST_DIR)/%.c $(OBJS_NO_MAIN) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< $(OBJS_NO_MAIN) -o $@ $(LDFLAGS)

# Run all test binaries
test: $(TARGET) $(TEST_BINS)
	@echo "Running Valgrind memory check on $(TARGET)..."
	@if $(MEM_TEST); then \
		echo "ALL MEMORY TESTS PASSED"; \
	else \
		echo "FAILED ATLEAST ONE MEMORY TEST"; \
		exit 1; \
	fi
	@for bin in $(TEST_BINS); do \
		echo "Running $$bin..."; \
		./$$bin || exit 1; \
	done

# Create build directories
$(OBJ_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -f log.txt

.PHONY: all clean test

