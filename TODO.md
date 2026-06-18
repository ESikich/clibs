<!--
TODO.md
Purpose: Project roadmap for low-level C libraries to build.
POSIX target: POSIX.1-2008 compatible C99.
Date modified: 2026-06-18.
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
- [x] `allocator`: stronger benchmark interpretation and benchmark docs.
- [x] `libc-mini`: tiny subset of libc: `memcpy`, `memmove`, `memset`,
  `memcmp`, `strlen`, `strcmp`, `strchr`, etc.
- [x] `test`: tiny unit test framework.
- [x] `bench`: reusable microbenchmark harness.
- [x] `array`: macro-generated typed dynamic arrays.

## Suggested First Batch

- [x] `libc-mini`: tiny subset of libc: `memcpy`, `memmove`, `memset`, `memcmp`,
  `strlen`, `strcmp`, `strchr`, etc.
- [x] `allocator`: arena allocator, bump allocator, checked interface, debug
  allocator.
- [x] `string-view`: non-owning string slices, trimming, splitting, parsing
  helpers.
- [x] `array`: typed or macro-based dynamic arrays.
- [x] `buffer`: byte buffers, growable arrays, ring buffers.
- [x] `hash`: hash functions plus hash table.
- [x] `file`: read/write whole file.
- [x] `file`: streaming file helpers.
- [x] `path`: path normalization, joining, basename/dirname.
- [x] `utf8`: validation, iteration, encoding/decoding.
- [x] `ascii`: classification and case conversion without locale.
- [x] `test`: tiny unit test framework.

## Core Runtime

- [x] `libc-mini`: tiny subset of libc: `memcpy`, `memmove`, `memset`, `memcmp`,
  `strlen`, `strcmp`, `strchr`, etc.
- [x] `allocator`: arena allocator, bump allocator, free-list allocator, pool
  allocator.
- [x] `string-view`: non-owning string slices, trimming, splitting, parsing
  helpers.
- [x] `buffer`: byte buffers, growable arrays, ring buffers.
- [x] `bitset`: fixed and dynamic bitsets, bit operations, bit scans.
- [x] `endian`: byte-order conversion, unaligned loads/stores.
- [x] `atomics`: thin wrappers around atomics or compiler intrinsics.
- [x] `time`: monotonic clock, duration math, timers.

## Data Structures

- [x] `array`: typed or macro-based dynamic arrays.
- [x] `list`: intrusive linked lists.
- [x] `queue`: allocation-free FIFO ring queues.
- [x] `priority-queue`: comparator-based priority queue.
- [x] `hash`: hash functions plus hash table.
- [x] `set`: hash set / bitset-backed set.
- [x] `map`: ordered map, maybe tree-based.
- [x] `heap`: binary heap.
- [ ] `trie`: byte trie or radix tree.
- [ ] `graph`: adjacency lists, traversal helpers.

## OS / Platform

- [ ] `sys`: portable wrappers for files, paths, environment, processes.
- [x] `file`: read/write whole file.
- [x] `file`: streaming file helpers.
- [x] `path`: path normalization, joining, basename/dirname.
- [ ] `mmap`: memory-mapped file wrapper.
- [ ] `thread`: thread creation, mutexes, condvars, thread-local storage.
- [ ] `event`: epoll/kqueue/select abstraction.
- [ ] `process`: spawn, wait, pipes.
- [ ] `signals`: safer signal handling utilities.

## Parsing / Encoding

- [x] `utf8`: validation, iteration, encoding/decoding.
- [x] `ascii`: classification and case conversion without locale.
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
- [x] `test`: tiny unit test framework.
- [x] `bench`: reusable microbenchmark harness.
- [ ] `trace`: scoped tracing/profiling events.
- [ ] `error`: error codes, error strings, result type conventions.

## Build Order Notes

Start with allocator, then immediately build reusable test and benchmark support
around it. Allocation policy is the foundation for strings, buffers, arrays,
hash maps, parsers, file readers, JSON, HTTP, and most later libraries.

Foundation work completed:

- [x] non-owning string views with trimming, splitting, comparisons, and checked
  integer parsing
- [x] fixed-size pool allocator
- [x] free-list allocator
- [x] debug checks for pool/free-list allocators
- [x] benchmarks that compare pool/free-list behavior against arena and system
  allocation
- [x] reusable unit test helper
- [x] reusable microbenchmark helper
- [x] macro-generated typed dynamic arrays with checked growth
- [x] growable byte buffers and caller-owned ring buffers
- [x] fixed-storage and allocator-backed bitsets with scans and boolean ops
- [x] non-owning byte-key hash table with FNV-1a helpers
- [x] POSIX whole-file read/write helpers
- [x] POSIX streaming file helpers
- [x] lexical POSIX path normalization, joining, basename, and dirname helpers
- [x] UTF-8 validation, iteration, encoding, and decoding helpers
- [x] locale-free ASCII classification, case conversion, and digit value helpers
- [x] byte-order conversion and unaligned fixed-width integer load/store helpers
- [x] compiler-atomic wrappers for integer and pointer state
- [x] monotonic clock, checked duration math, and elapsed timer helpers
- [x] allocation-free intrusive doubly linked lists
- [x] allocation-free fixed-capacity FIFO ring queues
- [x] allocation-free fixed-capacity comparator-based priority queues
- [x] allocator-backed ordered byte-key maps
- [x] non-owning byte-key hash set
- [x] allocation-free binary heap algorithms over caller-owned arrays
