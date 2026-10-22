CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -Iinclude
LIBS ?= -lm

SRC = src/dynamic_list.c src/token.c src/parser.c src/evaluator.c
OBJ = $(SRC:.c=.o)

BUILD_DIR = build

TARGET_CLI = $(BUILD_DIR)/engine_parser.exe
TARGET_TEST = $(BUILD_DIR)/run_tests.exe

.PHONY: all clean test

all: $(TARGET_TEST) $(TARGET_CLI)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET_TEST): $(SRC) tests/test_parser.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) tests/test_parser.c -o $@ $(LIBS)

$(TARGET_CLI): $(SRC) src/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) src/main.c -o $@ $(LIBS)

test: $(TARGET_TEST)
	./$(TARGET_TEST)

clean:
	rm -rf $(BUILD_DIR)
