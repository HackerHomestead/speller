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

| Type | Dependency | Package (Debian/Ubuntu) |
|------|------------|-------------------------|
| Build (required) | C++17 compiler, CMake 3.14+ | `build-essential`, `cmake` |
| Build (optional) | Hunspell (spell engine) | `libhunspell-dev` |
| Build (optional) | Readline (REPL autocomplete) | `libreadline-dev` |
| Runtime | Hunspell dictionaries | `hunspell-en-us` or `--dict-dir` |

**Full list with Fedora/Arch package names:** [DEPENDENCIES.md](DEPENDENCIES.md)

---

## 3. Command-Line Options

| Option | Description |
|--------|-------------|
| `--interactive` | Interactive word-by-word mode (default) |
| `--stream` | Stream mode: read from stdin, write to stdout |
| `--file PATH` | Read from file (implies stream mode) |
| `--check WORD` | Check a single word and show suggestions |
| `--dict-dir PATH` | Directory containing `.aff` and `.dic` dictionary files |
| `--user-dict PATH` | User dictionary (one word per line) |
| `--defs PATH` | Path to definition database (Phase 4) |
| `--fast` | In stream mode: auto-apply top suggestion |
| `--careful` | In stream mode: prompt when confidence is low (default) |
| `-h`, `--help` | Show help and exit |
| `-V`, `--version` | Show version and build info (Hunspell enabled/disabled) |

### Examples

```bash
spell                          # Interactive REPL (default)
spell --dict-dir ../data/dict  # REPL with bundled English dict
spell --check helo --dict-dir /usr/share/hunspell   # Quick word check
spell --check word --dict-dir /opt/dict --user-dict ~/user.dic
spell --stream                 # Stream from stdin
spell --file document.txt      # Spell-check a file
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

## 5. Interactive REPL Mode

Run `spell` with no arguments to enter the REPL:

```
spell - Interactive spell checker (type 'help' or '?' for help)

spell> hello
OK

spell> helo
Did you mean: hello?

spell> help
(shows help with examples)

spell> :q
(exits)
```

### REPL Commands

| Command | Action |
|---------|--------|
| `help`, `?` | Show help with examples |
| `quit`, `exit`, `:q` | Exit the REPL |

### REPL Tips

- **Tab** completes commands (when built with readline)
- **Up/Down** for command history (when built with readline)
- Type a word and press Enter to check spelling
- Empty line does nothing

---

## 6. Stream Mode

*(Phase 5 — not yet implemented)*

- **Input**: stdin or `--file`
- **Output**: Corrected text to stdout
- **--fast**: Automatically apply top suggestion
- **--careful**: Prompt when the engine is uncertain

---

## 7. User Dictionary

Custom words can be added to a user dictionary so they are treated as correct. Use `--user-dict PATH` to specify a file with one word per line. Lines starting with `#` are ignored.

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

### "Could not load dictionary"
- Install **libhunspell-dev** (build) and **hunspell-en-us** (runtime)
- Run `cmake ..` and check output: `Hunspell: YES` means spell check is enabled

### Build fails
- See [DEPENDENCIES.md](DEPENDENCIES.md) for required packages
- Ensure CMake 3.14+ and a C++17 compiler are installed

---

## 10. Version History

See [CHANGELOG.md](../CHANGELOG.md) for release notes.
