# Glossary build and format

The spell checker shows short definitions when you type a correct word or see "Did you mean …?" if a **glossary file** is provided. Definitions give **context clues** so users can tell similar-looking words apart (e.g. *there* vs *their*) and choose the correct meaning. See **[DICTIONARIES_AND_SOURCES.md](DICTIONARIES_AND_SOURCES.md)** for why coverage matters and which open-source dictionary/glossary sources we use.

The glossary is a plain-text file: one definition per line.

## Format

```
word<TAB>part_of_speech<TAB>short definition
```

- **word** — Headword (lowercase in lookups).
- **part_of_speech** — Optional; e.g. `n.` `v.` `adj.` `adv.` `prep.` `conj.` `interj.`
- **short definition** — One phrase; no newlines.

Lines starting with `#` are comments. Empty lines are ignored.

Example:

```
hello	interj.	used as a greeting
word	n.	a unit of language with meaning
```

## How the glossary is built

The project can build or update the glossary from a word list (e.g. the bundled dictionary) using three sources in order:

1. **WordNet** (NLTK) — Primary. Good coverage for common English; short glosses.
2. **Wiktionary** (API) — Fills gaps via the English Wiktionary API.
3. **LLM** (Ollama or OpenAI) — Optional; fills remaining gaps (e.g. rare or new words).

### Scripts

| Script | Purpose |
|--------|--------|
| **scripts/build_glossary.py** | Build `data/glossary.txt` from a word list (WordNet → Wiktionary → LLM). |
| **scripts/update_dict_glossary.py** | Optionally fetch dictionary (LibreOffice en_US), then run build_glossary. |
| **scripts/glossary_wordnet.py** | Standalone WordNet → glossary lines (for debugging or custom pipelines). |
| **scripts/glossary_wiktionary.py** | Standalone Wiktionary API → glossary lines. |
| **scripts/glossary_llm.py** | Standalone LLM → glossary lines (Ollama or OpenAI). |

### Quick build (WordNet only)

```bash
pip install nltk
python -c "import nltk; nltk.download('wordnet'); nltk.download('omw-1.4')"
pip install -r scripts/requirements-glossary.txt
python scripts/build_glossary.py --no-wiktionary
```

Uses `data/dict/en_US.dic` as the word list and writes `data/glossary.txt`. LLM is off by default; use `--llm` to enable.

### Full build (WordNet + Wiktionary; LLM optional)

```bash
pip install -r scripts/requirements-glossary.txt
python -c "import nltk; nltk.download('wordnet'); nltk.download('omw-1.4')"
python scripts/build_glossary.py
```

- **Wiktionary**: runs automatically for words missing from WordNet (requires `requests`).
- **LLM**: disabled by default. To fill remaining gaps with an LLM, add `--llm` and set **OLLAMA_MODEL** (e.g. `llama3.2`) with `ollama` installed, or **OPENAI_API_KEY** for OpenAI (or compatible) API.

### Update dictionary and glossary

```bash
python scripts/update_dict_glossary.py --fetch-dict
```

Downloads en_US.aff and en_US.dic from LibreOffice dictionaries into `data/dict/` (backs up existing files), then rebuilds `data/glossary.txt` from the new word list.

Without `--fetch-dict`, only rebuilds the glossary from the existing `data/dict/en_US.dic`.

## Open sources used

- **WordNet** — Princeton; WordNet license (permissive). [wordnet.princeton.edu](https://wordnet.princeton.edu/)
- **Wiktionary** — CC BY-SA. [en.wiktionary.org](https://en.wiktionary.org/)
- **LibreOffice dictionaries** — MPL 2.0. [github.com/LibreOffice/dictionaries](https://github.com/LibreOffice/dictionaries)

## Coverage tests

- **Overall**: At least **80%** of the words in the bundled `data/dict/en_US.dic` must have at least one entry in `data/glossary.txt`.
- **Confusables**: For commonly confused word pairs (e.g. *there*/*their*, *where*/*were*), **both** words must have definitions so the UI can show context for each suggestion. See [DICTIONARIES_AND_SOURCES.md](DICTIONARIES_AND_SOURCES.md).

Run:

```bash
./build/spell_tests "[coverage]"
```

After rebuilding the glossary (or changing the dictionary), run the full test suite to confirm coverage still passes.
