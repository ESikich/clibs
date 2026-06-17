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
ARRAY_OBJ = $(BUILD_DIR)/cl_array.o
BUFFER_OBJ = $(BUILD_DIR)/cl_buffer.o
FILE_OBJ = $(BUILD_DIR)/cl_file.o
HASH_OBJ = $(BUILD_DIR)/cl_hash.o
LIBC_OBJ = $(BUILD_DIR)/cl_libc.o
SV_OBJ = $(BUILD_DIR)/cl_sv.o
EXAMPLE_OVERVIEW = $(BUILD_DIR)/overview
TEST_ARRAY = $(BUILD_DIR)/test_array
TEST_ALLOC = $(BUILD_DIR)/test_alloc
TEST_BUFFER = $(BUILD_DIR)/test_buffer
TEST_FILE = $(BUILD_DIR)/test_file
TEST_HASH = $(BUILD_DIR)/test_hash
TEST_LIBC = $(BUILD_DIR)/test_libc
TEST_SV = $(BUILD_DIR)/test_sv
BENCH_ALLOC = $(BUILD_DIR)/bench_alloc

all: $(TEST_ALLOC) $(TEST_ARRAY) $(TEST_BUFFER) $(TEST_FILE) $(TEST_HASH) $(TEST_LIBC) $(TEST_SV) $(BENCH_ALLOC) $(EXAMPLE_OVERVIEW)

$(ALLOC_OBJ): src/cl_alloc.c include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_alloc.c -o $@

$(ARRAY_OBJ): src/cl_array.c include/cl_array.h include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_array.c -o $@

$(BUFFER_OBJ): src/cl_buffer.c include/cl_buffer.h include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_buffer.c -o $@

$(FILE_OBJ): src/cl_file.c include/cl_file.h include/cl_buffer.h include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_file.c -o $@

$(HASH_OBJ): src/cl_hash.c include/cl_hash.h include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_hash.c -o $@

$(LIBC_OBJ): src/cl_libc.c include/cl_libc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_libc.c -o $@

$(SV_OBJ): src/cl_sv.c include/cl_sv.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_sv.c -o $@

$(TEST_ALLOC): tests/test_alloc.c $(ALLOC_OBJ) include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_alloc.c $(ALLOC_OBJ) -o $@

$(TEST_ARRAY): tests/test_array.c $(ARRAY_OBJ) $(ALLOC_OBJ) include/cl_array.h include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_array.c $(ARRAY_OBJ) $(ALLOC_OBJ) -o $@

$(TEST_BUFFER): tests/test_buffer.c $(BUFFER_OBJ) $(ALLOC_OBJ) include/cl_buffer.h include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_buffer.c $(BUFFER_OBJ) $(ALLOC_OBJ) -o $@

$(TEST_FILE): tests/test_file.c $(FILE_OBJ) $(BUFFER_OBJ) $(ALLOC_OBJ) include/cl_file.h include/cl_buffer.h include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_file.c $(FILE_OBJ) $(BUFFER_OBJ) $(ALLOC_OBJ) -o $@

$(TEST_HASH): tests/test_hash.c $(HASH_OBJ) $(ALLOC_OBJ) include/cl_hash.h include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_hash.c $(HASH_OBJ) $(ALLOC_OBJ) -o $@

$(TEST_LIBC): tests/test_libc.c $(LIBC_OBJ) include/cl_libc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_libc.c $(LIBC_OBJ) -o $@

$(TEST_SV): tests/test_sv.c $(SV_OBJ) include/cl_sv.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_sv.c $(SV_OBJ) -o $@

$(BENCH_ALLOC): bench/bench_alloc.c $(ALLOC_OBJ) include/cl_alloc.h include/cl_bench.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) bench/bench_alloc.c $(ALLOC_OBJ) -o $@

$(EXAMPLE_OVERVIEW): examples/overview.c $(ALLOC_OBJ) $(ARRAY_OBJ) $(BUFFER_OBJ) $(FILE_OBJ) $(HASH_OBJ) $(LIBC_OBJ) $(SV_OBJ) include/cl_alloc.h include/cl_array.h include/cl_buffer.h include/cl_file.h include/cl_hash.h include/cl_libc.h include/cl_sv.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) examples/overview.c $(ALLOC_OBJ) $(ARRAY_OBJ) $(BUFFER_OBJ) $(FILE_OBJ) $(HASH_OBJ) $(LIBC_OBJ) $(SV_OBJ) -o $@

test: FORCE $(TEST_ALLOC) $(TEST_ARRAY) $(TEST_BUFFER) $(TEST_FILE) $(TEST_HASH) $(TEST_LIBC) $(TEST_SV)
	$(TEST_ALLOC)
	$(TEST_ARRAY)
	$(TEST_BUFFER)
	$(TEST_FILE)
	$(TEST_HASH)
	$(TEST_LIBC)
	$(TEST_SV)

bench: FORCE $(BENCH_ALLOC)
	$(BENCH_ALLOC)

example: FORCE $(EXAMPLE_OVERVIEW)
	$(EXAMPLE_OVERVIEW)

clean: FORCE
	rm -rf $(BUILD_DIR)

FORCE:
