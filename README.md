# spell

Offline spell checker and lightweight definition tool for neurodivergent users (ADHD, dyslexia, dysgraphia). Designed for embedded Linux and the vibePDA project.

**MVP (v0.4.0):** Interactive REPL, single-word check, config-file defaults, and Hunspell bundled as a static library — no system spell libs required.

## Quick Start

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run

```bash
./spell                                           # Interactive REPL (default)
./spell --help                                    # Show options
./spell --check hello                            # Quick word check (uses bundled dict)
./spell_tests                                     # Run unit + functional tests
```

In the REPL: type a word to check, `help` for commands, `:q` to quit.

### Requirements

| Dependency | Required | Package (Debian/Ubuntu) |
|------------|----------|-------------------------|
| C++17 compiler | Yes | `build-essential` |
| CMake 3.14+ | Yes | `cmake` |
| Git | Yes (fetch deps) | `git` |
| Hunspell | Bundled (static) | Fetched at configure; no system install |
| Catch2 | Yes (auto-fetched) | — |
| Dictionaries | No (minimal en_US in `data/dict/`) | `hunspell-en-us` for full coverage |

**Full details:** [docs/DEPENDENCIES.md](docs/DEPENDENCIES.md)

## Features

| Feature | Status |
|---------|--------|
| CLI with --help | ✅ |
| `--check WORD` spell check | ✅ |
| Hunspell integration | ✅ |
| User dictionary (`--user-dict`) | ✅ |
| Definitions (glossary file, bold/color in terminal) | ✅ |
| Interactive REPL (default) | ✅ |
| Stream mode | 🔲 Phase 5 |
| Definition lookup | 🔲 Phase 4 |
| Full TUI | 🔲 Phase 5 |

## Documentation

- **[User Manual](docs/MANUAL.md)** — Complete usage guide
- **[Dependencies](docs/DEPENDENCIES.md)** — Build and runtime deps, package names
- **[Tests](tests/README.md)** — Unit, functional, and fuzz tests
- **[IDEA.md](IDEA.md)** — Design document
- **[PLAN.md](PLAN.md)** — Development plan

## License

See project root for license information.
