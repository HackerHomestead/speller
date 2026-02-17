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
./spell --help      # Show options
./spell             # Interactive mode (currently stub)
./spell_tests       # Run unit tests
```

### Requirements

- C++17 compiler (g++ or clang++)
- CMake 3.14+
- Catch2 (fetched automatically)

Optional for Phase 2+:
- Hunspell (`libhunspell-dev` on Debian/Ubuntu, `hunspell-devel` on Fedora)

## Features

| Feature | Status |
|---------|--------|
| CLI with --help | ✅ |
| Interactive mode (stub) | ✅ |
| Stream mode (stub) | ✅ |
| Hunspell spell checking | 🔲 Phase 2 |
| User dictionary | 🔲 Phase 2 |
| Definition lookup | 🔲 Phase 4 |
| Full TUI | 🔲 Phase 5 |

## Documentation

- **[User Manual](docs/MANUAL.md)** — Complete usage guide
- **[IDEA.md](IDEA.md)** — Design document
- **[PLAN.md](PLAN.md)** — Development plan

## License

See project root for license information.
