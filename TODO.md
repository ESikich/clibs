<!--
TODO.md
Purpose: Project roadmap for low-level C libraries to build.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-17.
-->

# TODO

Memory safety and performance are the primary constraints for every library in
this roadmap. Prefer explicit ownership, checked arithmetic, bounded APIs,
predictable allocation behavior, and tests/benchmarks that exercise failure
paths as well as fast paths.

## Current Focus

- [x] `allocator`: checked allocation interface, system allocator, arena allocator,
  debug allocator, tests, and benchmark harness.
- [x] `allocator`: fixed-size pool allocator.
- [x] `allocator`: free-list allocator.
- [ ] `allocator`: stronger benchmark interpretation and benchmark docs.
- [ ] `test`: tiny unit test framework.
- [ ] `bench`: reusable microbenchmark harness.

## Suggested First Batch

- [ ] `libc-mini`: tiny subset of libc: `memcpy`, `memmove`, `memset`, `memcmp`,
  `strlen`, `strcmp`, `strchr`, etc.
- [x] `allocator`: arena allocator, bump allocator, checked interface, debug
  allocator.
- [ ] `string-view`: non-owning string slices, trimming, splitting, parsing
  helpers.
- [ ] `array`: typed or macro-based dynamic arrays.
- [ ] `buffer`: byte buffers, growable arrays, ring buffers.
- [ ] `hash`: hash functions plus hash table.
- [ ] `file`: read/write whole file, streaming file helpers.
- [ ] `path`: path normalization, joining, basename/dirname.
- [ ] `utf8`: validation, iteration, encoding/decoding.
- [ ] `test`: tiny unit test framework.

## Core Runtime

- [ ] `libc-mini`: tiny subset of libc: `memcpy`, `memmove`, `memset`, `memcmp`,
  `strlen`, `strcmp`, `strchr`, etc.
- [x] `allocator`: arena allocator, bump allocator, free-list allocator, pool
  allocator.
- [ ] `string-view`: non-owning string slices, trimming, splitting, parsing
  helpers.
- [ ] `buffer`: byte buffers, growable arrays, ring buffers.
- [ ] `bitset`: fixed and dynamic bitsets, bit operations, bit scans.
- [ ] `endian`: byte-order conversion, unaligned loads/stores.
- [ ] `atomics`: thin wrappers around atomics or compiler intrinsics.
- [ ] `time`: monotonic clock, duration math, timers.

## Data Structures

- [ ] `array`: typed or macro-based dynamic arrays.
- [ ] `list`: intrusive linked lists.
- [ ] `queue`: FIFO queues, ring queues, priority queues.
- [ ] `hash`: hash functions plus hash table.
- [ ] `map`: ordered map, maybe tree-based.
- [ ] `set`: hash set / bitset-backed set.
- [ ] `heap`: binary heap.
- [ ] `trie`: byte trie or radix tree.
- [ ] `graph`: adjacency lists, traversal helpers.

## OS / Platform

- [ ] `sys`: portable wrappers for files, paths, environment, processes.
- [ ] `file`: read/write whole file, streaming file helpers.
- [ ] `path`: path normalization, joining, basename/dirname.
- [ ] `mmap`: memory-mapped file wrapper.
- [ ] `thread`: thread creation, mutexes, condvars, thread-local storage.
- [ ] `event`: epoll/kqueue/select abstraction.
- [ ] `process`: spawn, wait, pipes.
- [ ] `signals`: safer signal handling utilities.

## Parsing / Encoding

- [ ] `utf8`: validation, iteration, encoding/decoding.
- [ ] `ascii`: classification and case conversion without locale.
- [ ] `parse`: integer/float parsing with explicit error handling.
- [ ] `format`: lightweight `printf` alternative.
- [ ] `json`: small JSON tokenizer/parser/writer.
- [ ] `ini`: INI parser/writer.
- [ ] `csv`: CSV reader/writer.
- [ ] `base64`: encode/decode.
- [ ] `hex`: encode/decode.
- [ ] `varint`: compact integer encoding.

## Binary / Systems

- [ ] `crc`: CRC32/CRC64.
- [ ] `hashing`: FNV, Murmur, xxHash-style non-crypto hashes.
- [ ] `crypto-primitives`: SHA-256, HMAC, ChaCha20, etc. only if we are willing
  to be very careful.
- [ ] `compression`: simple RLE, LZSS, DEFLATE later.
- [ ] `elf`: ELF reader.
- [ ] `pe`: PE reader.
- [ ] `archive`: tar reader/writer.

## Networking

- [ ] `socket`: TCP/UDP wrappers.
- [ ] `addr`: IP address parsing/formatting.
- [ ] `dns`: DNS packet parser/client.
- [ ] `http`: HTTP/1.1 parser/client/server basics.
- [ ] `url`: URL parser.
- [ ] `tls`: wrapper layer first; full TLS from scratch is not an early goal.

## Debug / Tooling

- [ ] `log`: structured logging with levels/sinks.
- [ ] `assert`: assertions, panic handler, debug traps.
- [ ] `test`: tiny unit test framework.
- [ ] `bench`: microbenchmark harness.
- [ ] `trace`: scoped tracing/profiling events.
- [ ] `error`: error codes, error strings, result type conventions.

## Build Order Notes

Start with allocator, then immediately build reusable test and benchmark support
around it. Allocation policy is the foundation for strings, buffers, arrays,
hash maps, parsers, file readers, JSON, HTTP, and most later libraries.

The next allocator work should be:

- [x] fixed-size pool allocator
- [x] free-list allocator
- [x] debug checks for pool/free-list allocators
- [x] benchmarks that compare pool/free-list behavior against arena and system
  allocation
