# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- (Phase 2) User dictionary add/learn from interactive mode

## [0.4.1] - 2026-02-28 — Sentence Correction

### Added
- **Sentence correction**: Type multiple words (2+) in REPL to correct entire sentences interactively
- **Interactive word-by-word review**: Shows misspelled words with numbered suggestions, prompts for choice
- **Explicit `correct` command**: Use `correct SENTENCE` for clearer intent
- **Auto-detection**: Any multi-word input is automatically treated as a sentence

### Documentation
- **docs/MANUAL.md**: New "Sentence Correction" section with usage examples
- **PLAN.md**: Added "Future Enhancements" section documenting all correction options

### Build
- **Default Release build**: CMake now defaults to `Release` build type if not specified
- Binary size reduced from 1.4MB (debug) to ~800KB (Release + LTO + strip)

## [0.4.0] - 2026-02-17 — MVP

**MVP status:** Spell checker is feature-complete for core use: interactive REPL, single-word check, bundled dictionary, config-file defaults, and statically linked Hunspell with no system dependency.

### Added
- **Vendored Hunspell**: Hunspell is fetched at configure time (v1.7.2) and built as a static library; no system `libhunspell-dev` required. Enables static linking and works on minimal Linux installs.
- **Config file**: Defaults from `~/.config/spell/config` or `~/.spellrc` (`dict_dir`, `user_dict`, `max_suggestions`). CLI overrides config.
- **Dict path parsing**: `--dict-dir` and REPL `load` accept either a directory or a path to a `.aff` file; shared logic in `util/dict_path`.
- **Clearer messages**: When built without Hunspell (N/A now that it’s bundled) or when dictionary fails to load, messages point to config file and `--dict-dir`.

### Changed
- Hunspell is always enabled (bundled); removed optional `find_package(Hunspell)`.
- REPL startup parses initial `dict_dir` from config (e.g. `dict_dir=/path/to/en_US.aff`).

### Documentation
- **docs/MANUAL.md**: New section “How the dictionary is loaded” (default, config file, CLI); troubleshooting for “no dictionary” and config.
- **docs/DEPENDENCIES.md**: Hunspell described as bundled (static); quick-install no longer requires `libhunspell-dev`.
- **docs/spell-config.example**: Example config for `~/.config/spell/config`.
- **README.md**, **PLAN.md**, **tests/README.md**: Updated for vendored Hunspell and MVP.

## [0.3.1] - 2025-02-17

### Added
- **Default dictionary**: Bundled `en_US` (`data/dict/`) used when no `--dict-dir`
- **REPL `load PATH`**: Load dictionary from path (e.g. `load /usr/share/hunspell`)
- **REPL `:load PATH`**: Same as load
- **REPL `dict`**: Show current dictionary path

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
