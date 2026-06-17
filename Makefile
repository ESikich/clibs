# Makefile
# Purpose: Build, test, and benchmark the clibs project.
# POSIX target: POSIX.1-2008 compatible C99.
# Date modified: 2026-06-17.

.POSIX:

CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -O2
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -Iinclude
BUILD_DIR = build

ALLOC_OBJ = $(BUILD_DIR)/cl_alloc.o
TEST_ALLOC = $(BUILD_DIR)/test_alloc
BENCH_ALLOC = $(BUILD_DIR)/bench_alloc

all: $(TEST_ALLOC) $(BENCH_ALLOC)

$(ALLOC_OBJ): src/cl_alloc.c include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_alloc.c -o $@

$(TEST_ALLOC): tests/test_alloc.c $(ALLOC_OBJ) include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_alloc.c $(ALLOC_OBJ) -o $@

$(BENCH_ALLOC): bench/bench_alloc.c $(ALLOC_OBJ) include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) bench/bench_alloc.c $(ALLOC_OBJ) -o $@

test: FORCE $(TEST_ALLOC)
	$(TEST_ALLOC)

bench: FORCE $(BENCH_ALLOC)
	$(BENCH_ALLOC)

clean: FORCE
	rm -rf $(BUILD_DIR)

FORCE:
