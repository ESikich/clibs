# Makefile
# Purpose: Build, test, and benchmark the clibs project.
# POSIX target: POSIX.1-2008 compatible C99.
# Date modified: 2026-06-18.

.POSIX:

CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -O2
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -Iinclude
BUILD_DIR = build

ALLOC_OBJ = $(BUILD_DIR)/cl_alloc.o
ATOMIC_OBJ = $(BUILD_DIR)/cl_atomic.o
ARRAY_OBJ = $(BUILD_DIR)/cl_array.o
ASCII_OBJ = $(BUILD_DIR)/cl_ascii.o
BITSET_OBJ = $(BUILD_DIR)/cl_bitset.o
BUFFER_OBJ = $(BUILD_DIR)/cl_buffer.o
ENDIAN_OBJ = $(BUILD_DIR)/cl_endian.o
FILE_OBJ = $(BUILD_DIR)/cl_file.o
HASH_OBJ = $(BUILD_DIR)/cl_hash.o
LIBC_OBJ = $(BUILD_DIR)/cl_libc.o
LIST_OBJ = $(BUILD_DIR)/cl_list.o
PATH_OBJ = $(BUILD_DIR)/cl_path.o
PRIORITY_QUEUE_OBJ = $(BUILD_DIR)/cl_priority_queue.o
QUEUE_OBJ = $(BUILD_DIR)/cl_queue.o
SV_OBJ = $(BUILD_DIR)/cl_sv.o
TIME_OBJ = $(BUILD_DIR)/cl_time.o
UTF8_OBJ = $(BUILD_DIR)/cl_utf8.o
EXAMPLE_OVERVIEW = $(BUILD_DIR)/overview
TEST_ARRAY = $(BUILD_DIR)/test_array
TEST_ALLOC = $(BUILD_DIR)/test_alloc
TEST_ATOMIC = $(BUILD_DIR)/test_atomic
TEST_BUFFER = $(BUILD_DIR)/test_buffer
TEST_ENDIAN = $(BUILD_DIR)/test_endian
TEST_ASCII = $(BUILD_DIR)/test_ascii
TEST_BITSET = $(BUILD_DIR)/test_bitset
TEST_FILE = $(BUILD_DIR)/test_file
TEST_HASH = $(BUILD_DIR)/test_hash
TEST_LIBC = $(BUILD_DIR)/test_libc
TEST_LIST = $(BUILD_DIR)/test_list
TEST_PATH = $(BUILD_DIR)/test_path
TEST_PRIORITY_QUEUE = $(BUILD_DIR)/test_priority_queue
TEST_QUEUE = $(BUILD_DIR)/test_queue
TEST_SV = $(BUILD_DIR)/test_sv
TEST_TIME = $(BUILD_DIR)/test_time
TEST_UTF8 = $(BUILD_DIR)/test_utf8
BENCH_ALLOC = $(BUILD_DIR)/bench_alloc

all: $(TEST_ALLOC) $(TEST_ARRAY) $(TEST_ASCII) $(TEST_ATOMIC) $(TEST_BITSET) $(TEST_BUFFER) $(TEST_ENDIAN) $(TEST_FILE) $(TEST_HASH) $(TEST_LIBC) $(TEST_LIST) $(TEST_PATH) $(TEST_PRIORITY_QUEUE) $(TEST_QUEUE) $(TEST_SV) $(TEST_TIME) $(TEST_UTF8) $(BENCH_ALLOC) $(EXAMPLE_OVERVIEW)

$(ALLOC_OBJ): src/cl_alloc.c include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_alloc.c -o $@

$(ATOMIC_OBJ): src/cl_atomic.c include/cl_atomic.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_atomic.c -o $@

$(ARRAY_OBJ): src/cl_array.c include/cl_array.h include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_array.c -o $@

$(ASCII_OBJ): src/cl_ascii.c include/cl_ascii.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_ascii.c -o $@

$(BITSET_OBJ): src/cl_bitset.c include/cl_bitset.h include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_bitset.c -o $@

$(BUFFER_OBJ): src/cl_buffer.c include/cl_buffer.h include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_buffer.c -o $@

$(ENDIAN_OBJ): src/cl_endian.c include/cl_endian.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_endian.c -o $@

$(FILE_OBJ): src/cl_file.c include/cl_file.h include/cl_buffer.h include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_file.c -o $@

$(HASH_OBJ): src/cl_hash.c include/cl_hash.h include/cl_alloc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_hash.c -o $@

$(LIBC_OBJ): src/cl_libc.c include/cl_libc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_libc.c -o $@

$(LIST_OBJ): src/cl_list.c include/cl_list.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_list.c -o $@

$(PATH_OBJ): src/cl_path.c include/cl_path.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_path.c -o $@

$(PRIORITY_QUEUE_OBJ): src/cl_priority_queue.c include/cl_priority_queue.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_priority_queue.c -o $@

$(QUEUE_OBJ): src/cl_queue.c include/cl_queue.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_queue.c -o $@

$(SV_OBJ): src/cl_sv.c include/cl_sv.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_sv.c -o $@

$(TIME_OBJ): src/cl_time.c include/cl_time.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_time.c -o $@

$(UTF8_OBJ): src/cl_utf8.c include/cl_utf8.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/cl_utf8.c -o $@

$(TEST_ALLOC): tests/test_alloc.c $(ALLOC_OBJ) include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_alloc.c $(ALLOC_OBJ) -o $@

$(TEST_ARRAY): tests/test_array.c $(ARRAY_OBJ) $(ALLOC_OBJ) include/cl_array.h include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_array.c $(ARRAY_OBJ) $(ALLOC_OBJ) -o $@

$(TEST_ATOMIC): tests/test_atomic.c $(ATOMIC_OBJ) include/cl_atomic.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_atomic.c $(ATOMIC_OBJ) -o $@

$(TEST_ASCII): tests/test_ascii.c $(ASCII_OBJ) include/cl_ascii.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_ascii.c $(ASCII_OBJ) -o $@

$(TEST_BITSET): tests/test_bitset.c $(BITSET_OBJ) $(ALLOC_OBJ) include/cl_bitset.h include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_bitset.c $(BITSET_OBJ) $(ALLOC_OBJ) -o $@

$(TEST_BUFFER): tests/test_buffer.c $(BUFFER_OBJ) $(ALLOC_OBJ) include/cl_buffer.h include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_buffer.c $(BUFFER_OBJ) $(ALLOC_OBJ) -o $@

$(TEST_ENDIAN): tests/test_endian.c $(ENDIAN_OBJ) include/cl_endian.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_endian.c $(ENDIAN_OBJ) -o $@

$(TEST_FILE): tests/test_file.c $(FILE_OBJ) $(BUFFER_OBJ) $(ALLOC_OBJ) include/cl_file.h include/cl_buffer.h include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_file.c $(FILE_OBJ) $(BUFFER_OBJ) $(ALLOC_OBJ) -o $@

$(TEST_HASH): tests/test_hash.c $(HASH_OBJ) $(ALLOC_OBJ) include/cl_hash.h include/cl_alloc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_hash.c $(HASH_OBJ) $(ALLOC_OBJ) -o $@

$(TEST_LIBC): tests/test_libc.c $(LIBC_OBJ) include/cl_libc.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_libc.c $(LIBC_OBJ) -o $@

$(TEST_LIST): tests/test_list.c $(LIST_OBJ) include/cl_list.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_list.c $(LIST_OBJ) -o $@

$(TEST_PATH): tests/test_path.c $(PATH_OBJ) include/cl_path.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_path.c $(PATH_OBJ) -o $@

$(TEST_PRIORITY_QUEUE): tests/test_priority_queue.c $(PRIORITY_QUEUE_OBJ) include/cl_priority_queue.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_priority_queue.c $(PRIORITY_QUEUE_OBJ) -o $@

$(TEST_QUEUE): tests/test_queue.c $(QUEUE_OBJ) include/cl_queue.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_queue.c $(QUEUE_OBJ) -o $@

$(TEST_SV): tests/test_sv.c $(SV_OBJ) include/cl_sv.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_sv.c $(SV_OBJ) -o $@

$(TEST_TIME): tests/test_time.c $(TIME_OBJ) include/cl_time.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_time.c $(TIME_OBJ) -o $@

$(TEST_UTF8): tests/test_utf8.c $(UTF8_OBJ) include/cl_utf8.h include/cl_test.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_utf8.c $(UTF8_OBJ) -o $@

$(BENCH_ALLOC): bench/bench_alloc.c $(ALLOC_OBJ) include/cl_alloc.h include/cl_bench.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) bench/bench_alloc.c $(ALLOC_OBJ) -o $@

$(EXAMPLE_OVERVIEW): examples/overview.c $(ALLOC_OBJ) $(ARRAY_OBJ) $(ASCII_OBJ) $(ATOMIC_OBJ) $(BITSET_OBJ) $(BUFFER_OBJ) $(ENDIAN_OBJ) $(FILE_OBJ) $(HASH_OBJ) $(LIBC_OBJ) $(LIST_OBJ) $(PATH_OBJ) $(PRIORITY_QUEUE_OBJ) $(QUEUE_OBJ) $(SV_OBJ) $(TIME_OBJ) $(UTF8_OBJ) include/cl_alloc.h include/cl_array.h include/cl_ascii.h include/cl_atomic.h include/cl_bitset.h include/cl_buffer.h include/cl_endian.h include/cl_file.h include/cl_hash.h include/cl_libc.h include/cl_list.h include/cl_path.h include/cl_priority_queue.h include/cl_queue.h include/cl_sv.h include/cl_time.h include/cl_utf8.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) examples/overview.c $(ALLOC_OBJ) $(ARRAY_OBJ) $(ASCII_OBJ) $(ATOMIC_OBJ) $(BITSET_OBJ) $(BUFFER_OBJ) $(ENDIAN_OBJ) $(FILE_OBJ) $(HASH_OBJ) $(LIBC_OBJ) $(LIST_OBJ) $(PATH_OBJ) $(PRIORITY_QUEUE_OBJ) $(QUEUE_OBJ) $(SV_OBJ) $(TIME_OBJ) $(UTF8_OBJ) -o $@

test: FORCE $(TEST_ALLOC) $(TEST_ARRAY) $(TEST_ASCII) $(TEST_ATOMIC) $(TEST_BITSET) $(TEST_BUFFER) $(TEST_ENDIAN) $(TEST_FILE) $(TEST_HASH) $(TEST_LIBC) $(TEST_LIST) $(TEST_PATH) $(TEST_PRIORITY_QUEUE) $(TEST_QUEUE) $(TEST_SV) $(TEST_TIME) $(TEST_UTF8)
	$(TEST_ALLOC)
	$(TEST_ARRAY)
	$(TEST_ASCII)
	$(TEST_ATOMIC)
	$(TEST_BITSET)
	$(TEST_BUFFER)
	$(TEST_ENDIAN)
	$(TEST_FILE)
	$(TEST_HASH)
	$(TEST_LIBC)
	$(TEST_LIST)
	$(TEST_PATH)
	$(TEST_PRIORITY_QUEUE)
	$(TEST_QUEUE)
	$(TEST_SV)
	$(TEST_TIME)
	$(TEST_UTF8)

bench: FORCE $(BENCH_ALLOC)
	$(BENCH_ALLOC)

example: FORCE $(EXAMPLE_OVERVIEW)
	$(EXAMPLE_OVERVIEW)

clean: FORCE
	rm -rf $(BUILD_DIR)

FORCE:
