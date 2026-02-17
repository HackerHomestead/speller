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

## Bundled (no install needed)

| Dependency | Purpose | How |
|------------|---------|-----|
| **Hunspell** | Spell checking engine | Fetched at configure time (Git tag v1.7.2), built as a **static library** and linked into `spell`. No system `libhunspell` required. |
| **Catch2** | Unit test framework | FetchContent (see below). |

## Build Dependencies (Optional)

| Dependency | Purpose | Package Names | If Missing |
|------------|---------|---------------|------------|
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
| **Hunspell** | Spell engine (static lib) | v1.7.2 |
| **Catch2** | Unit test framework | v3.5.2 |

---

## Quick Install (Debian/Ubuntu)

```bash
# Required (Hunspell is bundled; no libhunspell-dev needed)
sudo apt install build-essential cmake git

# Optional: REPL enhancements
sudo apt install libreadline-dev

# Optional: system dictionaries (project includes minimal en_US in data/dict/)
sudo apt install hunspell-en-us
```

---

## Quick Install (Fedora/RHEL)

```bash
# Required
sudo dnf install gcc-c++ cmake git

# Optional: REPL enhancements
sudo dnf install readline-devel

# Optional: system dictionaries
sudo dnf install hunspell-en-US
```

---

## Quick Install (Arch Linux)

```bash
# Required
sudo pacman -S base-devel cmake git

# Optional: REPL enhancements
sudo pacman -S readline

# Optional: system dictionaries
sudo pacman -S hunspell-en_us
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
  Hunspell:         bundled (static)
  Readline:         YES (autocomplete, history)   # or NO
  Catch2:           fetched automatically (tests)
----------------------------------------
  Spell checking:   enabled
```

Hunspell is always bundled (static); spell checking is always enabled in the build.

### At runtime

```bash
./spell --version
```

Shows version and Hunspell status:

```
spell 0.2.0
  Hunspell: enabled
```
