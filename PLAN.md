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

## Project Structure (Current)

```
speller/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # Entry point, CLI, REPL dispatch
│   ├── spell/
│   │   ├── spell_engine.hpp
│   │   ├── hunspell_engine.hpp/cpp
│   │   ├── stub_spell_engine.hpp/cpp
│   │   ├── definition_store.hpp
│   │   ├── stub_definition_store.hpp/cpp
│   │   ├── suggestion_orchestrator.hpp/cpp
│   │   └── repl.hpp/cpp       # Interactive REPL (default)
│   └── util/
│       └── config.hpp/cpp
├── data/dict/
│   ├── en_US.aff, en_US.dic   # Bundled English dict
│   └── README.md
├── docs/
│   ├── MANUAL.md
│   └── DEPENDENCIES.md
├── tests/
│   ├── test_*.cpp
│   ├── fuzz_utils.hpp/cpp
│   ├── fixtures/user.dic
│   └── README.md
├── IDEA.md
├── PLAN.md
└── CHANGELOG.md
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

**Deliverable:** `./spell --help` works; `./spell` enters REPL. ✅

---

## Phase 2: Spell Engine Integration

**Goal:** Real spell checking via Hunspell.

### Tasks

| # | Task | Status |
|---|------|--------|
| 2.1 | Implement `HunspellEngine` (load .aff/.dic, `is_correct`, `suggest`) | ✅ |
| 2.2 | Wire dictionary loading from `--dict-dir` or default path | ✅ |
| 2.3 | Add user dictionary support (e.g. `user.dic` in config dir) | ✅ |
| 2.4 | Graceful degradation when dict files missing | ✅ |
| 2.5 | Unit tests for `HunspellEngine` with sample dict | ✅ |

**Deliverable:** Can check words and get suggestions from Hunspell. ✅

---

## Phase 3: Suggestion Orchestration

**Goal:** Merge and rank suggestions from engines.

### Tasks

| # | Task | Status |
|---|------|--------|
| 3.1 | Implement `SuggestionOrchestrator` (merge, dedupe) | ✅ |
| 3.2 | Add Levenshtein edit-distance scoring | ⬜ |
| 3.3 | Add optional frequency-based scoring (frequency list) | ⬜ |
| 3.4 | Config: `--suggestions N` (default 5) | ⬜ |
| 3.5 | Tests for common typo patterns | ✅ (fuzz tests) |

**Deliverable:** Ranked "Did you mean...?" suggestions. ✅ (basic merge)

---

## Phase 4: Definition Store

**Goal:** Lookup short definitions for words.

### Tasks

| # | Task | Status |
|---|------|--------|
| 4.1 | Design minimal definition DB format (binary or simple KV) | ✅ (glossary: word\\tpos\\tdef) |
| 4.2 | Implement `MinimalDefinitionStore` (or stub with empty defs) | ✅ (FileDefinitionStore + stub) |
| 4.3 | Script/tool to build DB from WordNet or word list + glosses | ✅ (scripts/build_glossary.py) |
| 4.4 | Wire `--defs path` into config | ✅ |
| 4.5 | Graceful degradation when defs DB missing | ✅ |

**Deliverable:** `DefinitionStore::lookup(word)` returns definitions when DB present. ✅

### Glossary build pipeline

Scripts in **scripts/** build or update **data/glossary.txt** from a word list (e.g. **data/dict/en_US.dic**):

1. **WordNet** (NLTK) — primary definitions.
2. **Wiktionary** (API) — fill gaps.
3. **LLM** (Ollama or OpenAI) — optional, for remaining gaps.

See **[docs/GLOSSARY.md](docs/GLOSSARY.md)** for format, usage, and **scripts/update_dict_glossary.py** for dictionary + glossary update.

---

## Phase 5: TUI and Accessibility

**Goal:** Interactive and stream modes with neurodivergent-friendly UX.

### Tasks

| # | Task | Status |
|---|------|--------|
| 5.1 | REPL mode (default when no args) | ✅ |
| 5.2 | REPL: help, quit, autocomplete (readline), history | ✅ |
| 5.3 | Word-by-word mode: 1–5 (pick), s (skip), u (undo), d (define) | ⬜ |
| 5.4 | Implement stream mode (stdin/file → stdout) | ⬜ |
| 5.5 | Config: `--fast` vs `--careful` (auto-apply vs prompt) | ⬜ |
| 5.6 | Accessibility: verbosity, spacing, high-contrast options | ⬜ |
| 5.7 | Use ANSI terminal; add ncurses only if needed | ⬜ |

**Deliverable:** Full interactive and stream workflows. ✅ (REPL)

---

## Phase 6: Optimization & Embedded Tuning

**Goal:** Fit embedded constraints.

### Tasks

| # | Task | Status |
|---|------|--------|
| 6.1 | Measure memory/CPU on target hardware | ⬜ |
| 6.2 | Build with LTO, strip, static linking options | ✅ (Hunspell static) |
| 6.3 | Trim dictionary/definition sizes if needed | ⬜ |

---

## Phase 7: Documentation & Release

**Goal:** User and developer docs.

### Tasks

| # | Task | Status |
|---|------|--------|
| 7.1 | User-facing README (usage, options, examples) | ✅ |
| 7.2 | docs/MANUAL.md, docs/DEPENDENCIES.md | ✅ |
| 7.3 | tests/README.md, CHANGELOG.md | ✅ |
| 7.4 | Developer docs (build, extend, integrate) | ⬜ |

---

## Phase 8: Distribution & Packaging

**Goal:** Create redistributable builds for sharing.

### Tasks

| # | Task | Status |
|---|------|--------|
| 8.1 | Create release build script (build-release.sh) | ⬜ |
| 8.2 | Bundle dictionary + glossary into tarball/zip | ⬜ |
| 8.3 | Static build with all dependencies (Hunspell, readline) | ⬜ |
| 8.4 | Build for multiple architectures (x86_64, ARM) | ⬜ |
| 8.5 | Create install script or package (deb, rpm, AppImage) | ⬜ |

---

## Build Dependencies

See **[docs/DEPENDENCIES.md](docs/DEPENDENCIES.md)** for full details and package names.

- **Required**: C++17 compiler, CMake 3.14+, Git (for FetchContent)
- **Bundled**: Hunspell (FetchContent, built as static lib — no system install)
- **Optional**: Readline (libreadline-dev) — REPL autocomplete, history
- **FetchContent**: Hunspell v1.7.2, Catch2 (auto-fetched)

Optional later: ncurses, SQLite (for definition DB).

---

## MVP (v0.4.0)

Core spell checker is **MVP complete**: REPL by default, `--check`, config file, bundled dictionary, Hunspell statically linked.

---

## Next Steps

1. **Phase 3.2–3.4:** Levenshtein scoring, frequency list, `--suggestions N`
2. **Phase 5.3–5.6:** Word-by-word mode, stream mode, accessibility
3. **Phase 8:** Distribution & packaging for redistributable builds
4. **Glossary:** Run `scripts/build_glossary.py` or `scripts/update_dict_glossary.py` to refresh definitions; see docs/GLOSSARY.md
