# spell — User Manual

## 1. Introduction

**spell** is an offline spell checker and definition tool designed for neurodivergent users. It runs in the terminal and supports:

- **Interactive mode** — Step through text word-by-word, choose corrections, skip, undo, or look up definitions
- **Stream mode** — Read from stdin or a file, output corrected text to stdout

The tool is offline-first, suitable for embedded Linux devices, and uses Hunspell dictionaries.

---

## 2. Installation

### Build from Source

```bash
git clone <repository-url>
cd speller
mkdir build && cd build
cmake ..
cmake --build .
```

The `spell` executable will be in `build/`. Optionally install:

```bash
sudo cmake --install .   # If install rules are configured
```

### Dependencies

- **Build**: C++17 compiler, CMake 3.14+
- **Runtime**: Hunspell dictionaries (`.aff` and `.dic` files) — see Section 4

---

## 3. Command-Line Options

| Option | Description |
|--------|-------------|
| `--interactive` | Interactive word-by-word mode (default) |
| `--stream` | Stream mode: read from stdin, write to stdout |
| `--file PATH` | Read from file (implies stream mode) |
| `--dict-dir PATH` | Directory containing `.aff` and `.dic` dictionary files |
| `--defs PATH` | Path to definition database (Phase 4) |
| `--fast` | In stream mode: auto-apply top suggestion |
| `--careful` | In stream mode: prompt when confidence is low (default) |
| `-h`, `--help` | Show help and exit |

### Examples

```bash
spell                          # Interactive mode (default)
spell --stream                 # Stream from stdin
spell --file document.txt      # Spell-check a file
spell --dict-dir /opt/dict     # Use custom dictionary directory
spell --help                   # Show all options
```

---

## 4. Dictionaries

spell uses Hunspell dictionary files:

- **`.aff`** — Affix and rule file
- **`.dic`** — Word list

Place them in a directory and pass it with `--dict-dir`. Example:

```
/opt/dict/
  en_US.aff
  en_US.dic
```

Common sources:
- System: `/usr/share/hunspell/` (Linux)
- Debian/Ubuntu: `hunspell-en-us`, `hunspell-en-gb`, etc.

---

## 5. Interactive Mode

*(Phase 5 — not yet implemented)*

In interactive mode you step through text word-by-word:

| Key | Action |
|-----|--------|
| `1`–`5` | Pick suggestion 1–5 |
| `s` | Skip word |
| `u` | Undo previous change |
| `d` | Show definition |
| `q` | Quit (optionally save partial result) |

---

## 6. Stream Mode

*(Phase 5 — not yet implemented)*

- **Input**: stdin or `--file`
- **Output**: Corrected text to stdout
- **--fast**: Automatically apply top suggestion
- **--careful**: Prompt when the engine is uncertain

---

## 7. User Dictionary

*(Phase 2 — planned)*

Custom words can be added to a user dictionary (e.g. `user.dic`) so they are treated as correct. Location is configurable.

---

## 8. Accessibility

The design prioritizes:

- Low cognitive load
- Simple, predictable keystrokes
- High-contrast, uncluttered layout
- Short definitions by default
- Configurable verbosity

---

## 9. Troubleshooting

### "Dictionary not found"
- Ensure `--dict-dir` points to a directory with `.aff` and `.dic` files
- Check file names match (e.g. `en_US.aff` and `en_US.dic`)

### Build fails
- Install `libhunspell-dev` (or equivalent) if Phase 2+ is enabled
- Ensure CMake 3.14+ and a C++17 compiler are installed

---

## 10. Version History

See [CHANGELOG.md](../CHANGELOG.md) for release notes.
