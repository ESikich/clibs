# AGENTS.md
# Purpose: Working rules for agents and contributors in this repository.
# POSIX target: POSIX.1-2008 compatible C99.
# Date modified: 2026-06-17.

# Project Priorities

Memory safety and performance are the top priorities. Prefer explicit ownership,
checked arithmetic, simple lifetime models, and APIs that make size/alignment
requirements visible at call sites.

# Portability

- Target POSIX.1-2008 compatible systems.
- Use C99 unless a project decision changes the language baseline.
- Keep feature-test macros explicit. The default build defines
  `_POSIX_C_SOURCE=200809L`.
- Avoid non-POSIX APIs in core libraries unless there is a documented fallback.
- Keep public headers usable from C++ by preserving `extern "C"` guards where
  applicable.

# File Headers

Every maintained source, header, build, test, benchmark, and documentation file
should start with a short header that includes:

- file name
- purpose
- POSIX target
- date modified

Update the date modified field whenever a file receives a meaningful change.

# Comments

Use comments to explain invariants, ownership rules, portability constraints, and
non-obvious safety checks. Avoid comments that simply repeat the next line of
code.

# Documentation

Documentation belongs in `docs/` and should be updated in the same change as the
code it describes. Keep library documentation focused on contracts, safety
properties, portability requirements, and examples.

# Roadmap Hygiene

Treat `TODO.md` as maintained project state, not loose notes. When work
completes, mark the matching item complete everywhere it appears, update any
"next work" notes that the change makes stale, and keep roadmap guidance concise
instead of adding prose that repeats nearby checklist items. Before committing,
scan `TODO.md` for contradictions between Current Focus, category sections, and
Build Order Notes.

# Git Workflow

Plain `git` is the default tool for local commits and pushes. Do not require the
GitHub CLI for ordinary commit/push requests, and do not call out missing `gh`
unless the user specifically asks for GitHub CLI behavior or PR automation.

# Verification

Run `make test` after code changes. Run `make bench` when allocator behavior or
performance-sensitive code changes. Sanitizers are encouraged where available;
if a sanitizer cannot run under the current environment, record that limitation
in the final work summary.
