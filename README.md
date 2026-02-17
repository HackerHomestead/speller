# spell

Offline spell checker and lightweight definition tool for neurodivergent users (ADHD, dyslexia, dysgraphia). Designed for embedded Linux and the vibePDA project.

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
./spell --check hello --dict-dir ../data/dict     # Quick word check
./spell_tests                                     # Run unit + functional tests
```

In the REPL: type a word to check, `help` for commands, `:q` to quit.

### Requirements

| Dependency | Required | Package (Debian/Ubuntu) |
|------------|----------|-------------------------|
| C++17 compiler | Yes | `build-essential` |
| CMake 3.14+ | Yes | `cmake` |
| Catch2 | Yes (auto-fetched) | — |
| Hunspell | No (spell check) | `libhunspell-dev` |
| Hunspell dicts | No (bundled in `data/dict/`) | `hunspell-en-us` for full coverage |

**Full details:** [docs/DEPENDENCIES.md](docs/DEPENDENCIES.md)

## Features

| Feature | Status |
|---------|--------|
| CLI with --help | ✅ |
| `--check WORD` spell check | ✅ |
| Hunspell integration | ✅ |
| User dictionary (`--user-dict`) | ✅ |
| Interactive mode (stub) | ✅ |
| Stream mode (stub) | ✅ |
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
