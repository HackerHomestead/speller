# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- (Phase 2) Hunspell integration
- (Phase 2) User dictionary support

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
