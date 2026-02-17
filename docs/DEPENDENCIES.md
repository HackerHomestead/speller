# Dependencies

This document lists all dependencies for building and running **spell**, with package names for common distributions.

---

## Build Dependencies (Required)

| Dependency | Purpose | Min Version | Package Names |
|------------|---------|-------------|---------------|
| **C++ compiler** | Compile C++17 code | GCC 7+ / Clang 5+ | `build-essential` (Debian/Ubuntu), `gcc-c++` (Fedora), `base-devel` (Arch) |
| **CMake** | Build system | 3.14+ | `cmake` |
| **Git** | Fetch Catch2 (build-time) | — | `git` |

---

## Build Dependencies (Optional)

| Dependency | Purpose | Package Names | If Missing |
|------------|---------|---------------|------------|
| **Hunspell** (libhunspell + headers) | Spell checking engine | `libhunspell-dev` (Debian/Ubuntu), `hunspell-devel` (Fedora/RHEL), `hunspell` (Arch) | Build succeeds; spell checking disabled at runtime |
| **Readline** | REPL autocomplete, history | `libreadline-dev` (Debian/Ubuntu), `readline-devel` (Fedora) | REPL works with plain getline (no Tab/Up/Down) |

---

## Runtime Dependencies

| Dependency | Purpose | Package Names | If Missing |
|------------|---------|---------------|------------|
| **Hunspell dictionaries** | Word lists for spell checking | `hunspell-en-us`, `hunspell-en-gb`, etc. (Debian/Ubuntu) | Project includes minimal English dict in `data/dict/`; use `--dict-dir` for full coverage |

---

## FetchContent (Automatic)

These are downloaded by CMake during configure; no manual install needed:

| Dependency | Purpose | Version |
|------------|---------|---------|
| **Catch2** | Unit test framework | v3.5.2 |

---

## Quick Install (Debian/Ubuntu)

```bash
# Required
sudo apt install build-essential cmake git

# Optional: spell checking + REPL enhancements
sudo apt install libhunspell-dev hunspell-en-us libreadline-dev
```

---

## Quick Install (Fedora/RHEL)

```bash
# Required
sudo dnf install gcc-c++ cmake git

# Optional: spell checking
sudo dnf install hunspell-devel hunspell-en-US
```

---

## Quick Install (Arch Linux)

```bash
# Required
sudo pacman -S base-devel cmake git

# Optional: spell checking
sudo pacman -S hunspell hunspell-en_us
```

---

## Verifying Dependencies

### At configure time (`cmake ..`)

The configure step prints a dependency summary:

```
spell 0.2.0 - Dependency check
----------------------------------------
  C++ compiler:    GNU 11.4.0
  CMake:            3.22.1 (min 3.14)
  Hunspell:         YES (libhunspell.so)   # or NO if missing
  Catch2:           fetched automatically (tests)
----------------------------------------
  Spell checking:   enabled   # or disabled
```

- **Hunspell: YES** — Spell checking will work at runtime
- **Hunspell: NO** — Build succeeds; `--check` will report "Could not load dictionary"

### At runtime

```bash
./spell --version
```

Shows version and whether Hunspell was built in:

```
spell 0.2.0
  Hunspell: enabled
```

or

```
spell 0.2.0
  Hunspell: disabled (install libhunspell-dev to enable)
```
