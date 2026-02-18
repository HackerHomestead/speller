# Tests

## Test Categories

| Tag | Description |
|-----|-------------|
| `[spell_engine]` | SpellEngine interface and StubSpellEngine |
| `[definition_store]` | DefinitionStore interface and StubDefinitionStore |
| `[suggestion_orchestrator]` | Suggestion merging and ranking |
| `[config]` | Config parsing |
| `[cli]` | CLI flags (--help, --version) |
| `[hunspell_engine]` | HunspellEngine (Hunspell is bundled; tests always run) |
| `[coverage]` | Definition coverage: glossary vs bundled en_US.dic (≥80%); confusables (both words in pair must have definitions) |
| `[functional]` | End-to-end spell binary tests |
| `[fuzz]` | Fuzz and stress tests |
| `[dyslexic]` | Dyslexic-like misspellings (actual typos) |
| `[fuzzing]` | Random fuzz strings (nonsense input) |

## Fuzz Tests

### Dyslexic Misspellings (`[fuzz][dyslexic]`)

100 words generated to simulate common dyslexia patterns:

- **Letter reversals**: b↔d, p↔q, m↔w, n↔u
- **Phonological**: c↔k, c↔s, g↔j, ph↔f
- **Transposition**: adjacent letter swap (e.g. "teh" from "the")
- **Doubling**: duplicate a letter
- **Omission**: drop a letter

These are **actual misspellings** derived from correct words. The test verifies:

1. No crash on any input
2. Suggestions often include the original correct word

### Fuzz Words (`[fuzz][fuzzing]`)

100 random strings (letters + digits, 2–12 chars). These are **nonsense words** with no intended correction. The test verifies the engine does not crash on arbitrary input.

## Running Tests

```bash
./spell_tests                    # All tests
./spell_tests "[fuzz]"           # Fuzz tests only
./spell_tests "[dyslexic]"      # Dyslexic misspelling tests
./spell_tests "[fuzzing]"       # Random fuzz tests
./spell_tests "[hunspell]"      # Hunspell engine tests
```
