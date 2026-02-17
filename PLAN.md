# Development Plan: vibePDA Spell & Definition Module

This plan breaks down the IDEA.md design into actionable development tasks. The project will be built as a **standalone binary** (`spell` or `vibe-spell`) first; vibePDA integration can be added later.

---

## Architecture Summary

```
┌─────────────────────────────────────────────────────────────────┐
│                         spell (main)                             │
│  CLI parsing → Config → run_spell()                              │
└─────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                         SpellTui                                 │
│  Interactive mode | Stream mode | Key handling | Display         │
└─────────────────────────────────────────────────────────────────┘
                                │
            ┌───────────────────┼───────────────────┐
            ▼                   ▼                   ▼
┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐
│ SuggestionOrch.  │  │  DefinitionStore  │  │   SpellEngine     │
│ merge, rank,     │  │  lookup(word)     │  │  is_correct()     │
│ score suggestions│  │  → Definition     │  │  suggest()        │
└──────────────────┘  └──────────────────┘  └──────────────────┘
            │                                           │
            └───────────────────┬───────────────────────┘
                                ▼
                    ┌──────────────────────┐
                    │   HunspellEngine      │
                    │   (.aff + .dic)       │
                    └──────────────────────┘
```

---

## Project Structure (Target)

```
speller/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # Entry point, CLI, dispatch
│   ├── spell/
│   │   ├── spell_engine.hpp/cpp
│   │   ├── hunspell_engine.hpp/cpp
│   │   ├── definition_store.hpp/cpp
│   │   ├── suggestion_orchestrator.hpp/cpp
│   │   └── spell_tui.hpp/cpp
│   └── util/
│       ├── config.hpp/cpp
│       └── text_utils.hpp/cpp   # word tokenization, etc.
├── include/
│   └── spell/                    # Public headers (if any)
├── data/                          # Default dict paths, scripts
│   └── dict/                      # Symlink or copy of hunspell dicts
├── tests/
│   └── ...
├── IDEA.md
└── PLAN.md
```

---

## Phase 1: Core Skeleton (Current Focus)

**Goal:** Define all interfaces, set up build system, produce a runnable stub.

### Tasks

| # | Task | Status |
|---|------|--------|
| 1.1 | Create CMake build system (C++17, find Hunspell) | ✅ |
| 1.2 | Define `Suggestion` struct and `SpellEngine` interface | ✅ |
| 1.3 | Define `Definition` struct and `DefinitionStore` interface | ✅ |
| 1.4 | Define `SuggestionOrchestrator` interface/skeleton | ✅ |
| 1.5 | Define `SpellTui` interface/skeleton | ⬜ (Phase 5) |
| 1.6 | Create `Config` struct (dict-dir, defs path, mode, etc.) | ✅ |
| 1.7 | Implement `main()` with CLI parsing (`--interactive`, `--file`, `--dict-dir`, etc.) | ✅ |
| 1.8 | Implement `run_spell(Config)` that prints placeholder and exits | ✅ |

**Deliverable:** `./spell --help` works; `./spell` prints "Spell module (stub)" or similar. ✅

---

## Phase 2: Spell Engine Integration

**Goal:** Real spell checking via Hunspell.

### Tasks

| # | Task | Status |
|---|------|--------|
| 2.1 | Implement `HunspellEngine` (load .aff/.dic, `is_correct`, `suggest`) | ⬜ |
| 2.2 | Wire dictionary loading from `--dict-dir` or default path | ⬜ |
| 2.3 | Add user dictionary support (e.g. `user.dic` in config dir) | ⬜ |
| 2.4 | Graceful degradation when dict files missing | ⬜ |
| 2.5 | Unit tests for `HunspellEngine` with sample dict | ⬜ |

**Deliverable:** Can check words and get suggestions from Hunspell.

---

## Phase 3: Suggestion Orchestration

**Goal:** Merge and rank suggestions from engines.

### Tasks

| # | Task | Status |
|---|------|--------|
| 3.1 | Implement `SuggestionOrchestrator` (merge, dedupe) | ⬜ |
| 3.2 | Add Levenshtein edit-distance scoring | ⬜ |
| 3.3 | Add optional frequency-based scoring (frequency list) | ⬜ |
| 3.4 | Config: `--suggestions N` (default 5) | ⬜ |
| 3.5 | Tests for common typo patterns | ⬜ |

**Deliverable:** Ranked "Did you mean...?" suggestions.

---

## Phase 4: Definition Store

**Goal:** Lookup short definitions for words.

### Tasks

| # | Task | Status |
|---|------|--------|
| 4.1 | Design minimal definition DB format (binary or simple KV) | ⬜ |
| 4.2 | Implement `MinimalDefinitionStore` (or stub with empty defs) | ⬜ |
| 4.3 | Script/tool to build DB from WordNet or word list + glosses | ⬜ |
| 4.4 | Wire `--defs path` into config | ⬜ |
| 4.5 | Graceful degradation when defs DB missing | ⬜ |

**Deliverable:** `DefinitionStore::lookup(word)` returns definitions when DB present.

---

## Phase 5: TUI and Accessibility

**Goal:** Interactive and stream modes with neurodivergent-friendly UX.

### Tasks

| # | Task | Status |
|---|------|--------|
| 5.1 | Implement interactive word-by-word mode (display, navigation) | ⬜ |
| 5.2 | Keybindings: 1–5 (pick), s (skip), u (undo), d (define), q (quit) | ⬜ |
| 5.3 | Implement stream mode (stdin/file → stdout) | ⬜ |
| 5.4 | Config: `--fast` vs `--careful` (auto-apply vs prompt) | ⬜ |
| 5.5 | Accessibility: verbosity, spacing, high-contrast options | ⬜ |
| 5.6 | Use ANSI terminal; add ncurses only if needed | ⬜ |

**Deliverable:** Full interactive and stream workflows.

---

## Phase 6: Optimization & Embedded Tuning

**Goal:** Fit embedded constraints.

### Tasks

| # | Task | Status |
|---|------|--------|
| 6.1 | Measure memory/CPU on target hardware | ⬜ |
| 6.2 | Build with LTO, strip, static linking options | ⬜ |
| 6.3 | Trim dictionary/definition sizes if needed | ⬜ |

---

## Phase 7: Documentation & Release

**Goal:** User and developer docs.

### Tasks

| # | Task | Status |
|---|------|--------|
| 7.1 | User-facing README (usage, options, examples) | ⬜ |
| 7.2 | Developer docs (build, extend, integrate) | ⬜ |

---

## Build Dependencies

- **Hunspell** (libhunspell-dev or hunspell-devel)
- **C++17** compiler (g++, clang++)
- **CMake** 3.14+

Optional later: ncurses, SQLite (for definition DB).

---

## Next Steps

1. **Start Phase 1.1:** Create `CMakeLists.txt` and minimal `main.cpp`.
2. **Phase 1.2–1.5:** Add header files with interfaces (no implementations yet).
3. **Phase 1.6–1.8:** Config, CLI, and stub `run_spell()`.
