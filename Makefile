CC  := gcc
OPT := -O0
INC := include
CFLAGS := $(OPT) -I$(INC) -Wall -Wextra -g -MMD -MP

CFLAGS_PERFT := -O2 -I$(INC) -DNDEBUG

BUILD_DIR := build
TOOL_DIR  := tool

SRC_DIR := src
OBJ_DIR := $(BUILD_DIR)/objects
BIN_DIR := $(BUILD_DIR)/bin

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
-include $(OBJ:.o=.d)

TEST_DIR     := tests
TEST_BIN_DIR := $(TEST_DIR)/bin
TEST_SRC  	 := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS 	 := $(patsubst $(TEST_DIR)/%.c, $(TEST_BIN_DIR)/%, $(TEST_SRC))
TEST_OBJ	 := $(filter-out $(OBJ_DIR)/main.o, $(OBJ))

MAGIC_SRC := $(TOOL_DIR)/gen_magics.c $(SRC_DIR)/board.c $(SRC_DIR)/attack.c
PERFT_SRC := $(TOOL_DIR)/run_perft.c $(SRC_DIR)/board.c $(SRC_DIR)/attack.c $(SRC_DIR)/move.c $(SRC_DIR)/magic.c

BINARY    := $(BIN_DIR)/chess
MAGIC_BIN := $(BIN_DIR)/magic
PERFT_BIN  := $(BIN_DIR)/perft

.PHONY: all clean test magic rebuild perft

.DEFAULT_GOAL := all

all: $(BINARY)
	$(BINARY)

perft: $(PERFT_BIN)
	$(PERFT_BIN)

clean:
	rm -rf $(BUILD_DIR) $(TEST_BIN_DIR) src/magic.c

rebuild:
	make clean && make magic && make test

test: $(TEST_BINS)
	for bin in $^; do $$bin || exit 1; done

magic: $(MAGIC_BIN)
	$(MAGIC_BIN)

$(MAGIC_BIN): $(MAGIC_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -DGENERATING_MAGICS $^ -o $@

$(PERFT_BIN): $(PERFT_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS_PERFT) $^ -o $@

$(TEST_BIN_DIR)/%: $(TEST_DIR)/%.c $(TEST_OBJ) | $(TEST_BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lcriterion

$(BINARY): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR) $(TEST_BIN_DIR):
	mkdir -p $@
