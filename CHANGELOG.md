# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- (Phase 2) User dictionary add/learn from interactive mode

## [0.3.0] - 2025-02-17

### Added
- **REPL by default**: `spell` with no args enters interactive REPL
- **help** and **?** commands with examples
- **quit**, **exit**, **:q** to exit
- **Autocomplete** (Tab) for commands when built with readline
- **History** (Up/Down) when built with readline
- Graceful fallback to plain getline when stdin is piped (no readline blocking)

## [0.2.1] - 2025-02-17

### Added
- Bundled English dictionary (`data/dict/en_US.aff`, `en_US.dic`) — ~80 words for testing
- Unit tests: SpellEngine (case, empty word, Suggestion struct), SuggestionOrchestrator (empty engines, deduplication), DefinitionStore (overwrite, empty)
- Functional tests: `--help`, `-h`, `--version`, `-V`, no-args, `--check` with bundled dict
- HunspellEngine user dictionary test (when Hunspell available)
- Test fixtures: `tests/fixtures/user.dic`
- **Fuzz tests**: 100 dyslexic-like misspellings + 100 random fuzz words
  - `[fuzz][dyslexic]`: Actual misspellings (reversals, transposition, doubling, omission)
  - `[fuzz][fuzzing]`: Random nonsense strings — no crash, no UB
  - With Hunspell: validates suggestions include correct word for dyslexic input

## [0.2.0] - 2025-02-17

### Added
- Phase 2: Hunspell integration
- `HunspellEngine` — SpellEngine implementation using Hunspell
- `--check WORD` — Quick single-word spell check
- `--user-dict PATH` — User dictionary (one word per line)
- Graceful degradation when Hunspell not available (libhunspell-dev)
- CMake fallback: find Hunspell via find_path/find_library when pkg-config fails
- Unit tests for HunspellEngine (run when dict available)

## [0.1.0] - 2025-02-17

### Added
- Phase 1: Core skeleton
- CMake build system (C++17, Catch2 for tests)
- `SpellEngine` interface with `StubSpellEngine` implementation
- `DefinitionStore` interface with `StubDefinitionStore` implementation
- `SuggestionOrchestrator` for merging and ranking suggestions from multiple engines
- `Config` struct with CLI argument parsing
- Command-line interface: `--interactive`, `--stream`, `--file`, `--dict-dir`, `--defs`, `--fast`, `--careful`, `--help`
- `run_spell()` stub entry point
- Unit tests for all core components (15 tests)
- CHANGELOG, README, and user manual
