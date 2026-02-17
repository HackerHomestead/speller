# spell — User Manual

## 1. Introduction

**spell** is an offline spell checker and definition tool designed for neurodivergent users. As of **v0.4.0 (MVP)**, it runs in the terminal with:

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

## 4. How the dictionary is loaded

You can set the dictionary in three ways (first one wins):

1. **Default** — If the project was built with a bundled dict, `spell` uses `data/dict` (en_US). So running `spell` or `spell --check hello` from the project root uses it without any options.

2. **Config file** — Put defaults in a file in your home directory so you don’t need to pass paths every time:
   - **`~/.config/spell/config`** (preferred), or
   - **`~/.spellrc`**

   Example `~/.config/spell/config`:

   ```
   dict_dir=/usr/share/hunspell
   user_dict=/home/me/.config/spell/user.dic
   ```

   You can use either a **directory** that contains `.aff` and `.dic` files, or the **path to a `.aff` file** (e.g. `dict_dir=/path/to/data/dict/en_US.aff`). Create the file and directory if needed:
   ```bash
   mkdir -p ~/.config/spell
   cp docs/spell-config.example ~/.config/spell/config
   # Edit ~/.config/spell/config and set dict_dir to your dict path
   ```

3. **Command line** — Override for one run:
   ```bash
   spell --dict-dir /usr/share/hunspell
   spell --dict-dir ./data/dict/en_US.aff
   spell --check hello --dict-dir data/dict
   ```

**Summary:** Use the config file for your usual dictionary and user dict; use `--dict-dir` when you want to point to a different path for a single run.

---

## 5. Dictionaries (file format)

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

## 6. Interactive REPL Mode

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
| `load PATH`, `:load PATH` | Load dictionary from PATH (dir with .aff, .dic) |
| `dict` | Show current dictionary path |
| `define WORD`, `def WORD` | Show definition for WORD (from glossary). After "Did you mean …?", **Tab** cycles through suggested words to complete `def` / `define`. |
| `quit`, `exit`, `:q` | Exit the REPL |

**Default:** Bundled `en_US` dictionary (`data/dict/`) is used when no `--dict-dir` is given.

### REPL Tips

- **Tab** completes commands (when built with readline)
- **Up/Down** for command history (when built with readline)
- Type a word and press Enter to check spelling
- Empty line does nothing

---

## 7. Stream Mode

*(Phase 5 — not yet implemented)*

- **Input**: stdin or `--file`
- **Output**: Corrected text to stdout
- **--fast**: Automatically apply top suggestion
- **--careful**: Prompt when the engine is uncertain

---

## 8. User Dictionary

Custom words can be added to a user dictionary so they are treated as correct. Use `--user-dict PATH` to specify a file with one word per line. Lines starting with `#` are ignored.

---

## 9. Definitions

When you type a correct word or see "Did you mean …?", **spell** can show a short definition if a glossary file is available.

- **Glossary file**: Use `--defs PATH` or set **defs_path** in your config file. The file format is one definition per line:  
  `word<TAB>part_of_speech<TAB>short definition`  
  or `word<TAB>definition` (part of speech optional). Lines starting with `#` are comments.
- **Default**: When built from source, a default path to `data/glossary.txt` is used if you don’t set one (so definitions work when you run from the project root).
- **Display**: In a terminal, the **word** is shown in **bold** and the definition text in color for easier reading. Set the environment variable **NO_COLOR** to disable colors.

Example (with glossary loaded):

```
spell> hello
OK
  hello (interj.) used as a greeting

spell> helo
Did you mean: hello, …?
  hello (interj.) used as a greeting
```

---

## 10. Accessibility

The design prioritizes:

- Low cognitive load
- Simple, predictable keystrokes
- High-contrast, uncluttered layout
- Short definitions by default
- Configurable verbosity

---

## 11. Troubleshooting

### "Dictionary not found"
- Ensure `--dict-dir` points to a directory with `.aff` and `.dic` files
- Check file names match (e.g. `en_US.aff` and `en_US.dic`)

### "(no dictionary)" or "Spell checking is disabled"
- If the message says **"this build was compiled without Hunspell"**: install **libhunspell-dev** (Debian/Ubuntu), then **rebuild** (`cmake .. && cmake --build .`). Config and `--dict-dir` cannot enable spell check when Hunspell was not linked at build time.
- Run `spell --version` and check for "Hunspell: enabled". If it says "disabled", rebuild with Hunspell installed.

### "Could not load dictionary" (Hunspell enabled but path wrong)
- Install **hunspell-en-us** (or put `.aff`/`.dic` in a directory) and set **dict_dir** in `~/.config/spell/config` or use `--dict-dir`
- Ensure the path points to a directory containing `.aff` and `.dic`, or to a `.aff` file

### Build fails
- See [DEPENDENCIES.md](DEPENDENCIES.md) for required packages
- Ensure CMake 3.14+ and a C++17 compiler are installed

---

## 12. Version History

- **0.4.0 (MVP)** — Bundled Hunspell (static), config file, REPL by default, single-word check. See [CHANGELOG.md](../CHANGELOG.md) for full release notes.
