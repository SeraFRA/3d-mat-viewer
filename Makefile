CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -Iinclude
MATH_LIBS ?= -lm
GL_LIBS ?= -lopengl32 -lglu32 -lgdi32 -luser32

CORE_SRC = src/dynamic_list.c src/token.c src/parser.c src/evaluator.c src/mesh.c src/marching_cubes.c

BUILD_DIR = build

TARGET_VIEWER = $(BUILD_DIR)/engine_3d.exe
TARGET_PARSER_TEST = $(BUILD_DIR)/run_tests.exe
TARGET_MC_TEST = $(BUILD_DIR)/run_mc_tests.exe

.PHONY: all clean test

all: $(TARGET_PARSER_TEST) $(TARGET_MC_TEST) $(TARGET_VIEWER)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET_PARSER_TEST): $(CORE_SRC) tests/test_parser.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CORE_SRC) tests/test_parser.c -o $@ $(MATH_LIBS)

$(TARGET_MC_TEST): $(CORE_SRC) tests/test_marching_cubes.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CORE_SRC) tests/test_marching_cubes.c -o $@ $(MATH_LIBS)

$(TARGET_VIEWER): $(CORE_SRC) src/viewer.c src/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CORE_SRC) src/viewer.c src/main.c -o $@ $(MATH_LIBS) $(GL_LIBS)

test: $(TARGET_PARSER_TEST) $(TARGET_MC_TEST)
	./$(TARGET_PARSER_TEST)
	./$(TARGET_MC_TEST)

clean:
	rm -rf $(BUILD_DIR)
