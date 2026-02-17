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
./spell --help                                    # Show options
./spell --check helo --dict-dir /usr/share/hunspell  # Check word (needs libhunspell-dev)
./spell_tests                                     # Run unit tests
```

### Requirements

- C++17 compiler (g++ or clang++)
- CMake 3.14+
- Catch2 (fetched automatically)

For spell checking:
- **libhunspell-dev** (Debian/Ubuntu) or **hunspell-devel** (Fedora)
- Hunspell dictionaries, e.g. **hunspell-en-us** (or use `--dict-dir`)

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
- **[IDEA.md](IDEA.md)** — Design document
- **[PLAN.md](PLAN.md)** — Development plan

## License

See project root for license information.
