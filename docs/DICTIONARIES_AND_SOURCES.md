# Dictionaries and glossary sources

Context clues (definitions) are important for users who may have difficulty telling similar-looking words apart. Showing a short definition next to each suggestion helps them choose the word with the **correct meaning** instead of a visually similar one. This document covers how we address coverage and which open-source sources we use or recommend.

---

## Why glossary coverage matters

- **Visually similar words**: e.g. *there* / *their* / *they're*, *where* / *were*, *right* / *write*. Without definitions, users may pick the wrong one.
- **Same sound, different meaning**: e.g. *to* / *too* / *two*. A brief gloss disambiguates.
- **Our approach**: Require that a high fraction of words in the spell-check dictionary have at least one glossary entry (see tests), and that commonly confused word pairs **both** have definitions so the UI can show context for every suggestion.

---

## Open-source dictionary and glossary sources

### Primary: WordNet (NLTK / Princeton)

- **What**: Lexical database — synsets, short glosses, part-of-speech.
- **Coverage**: ~117k synsets (Princeton 3.1); good for common English.
- **License**: WordNet license (permissive).
- **How we use it**: `scripts/glossary_wordnet.py` and `scripts/build_glossary.py` use NLTK’s `wordnet` to export `word\tpos\tgloss` for any word list.
- **Links**: [wordnet.princeton.edu](https://wordnet.princeton.edu/), [NLTK WordNet](https://www.nltk.org/howto/wordnet.html).

### Open English WordNet (OEWN)

- **What**: Open, community WordNet (fork of Princeton); updated yearly.
- **Coverage**: ~162k words (2025 Plus); LMF, JSON, RDF, WNDB.
- **License**: CC-BY 4.0.
- **Use**: Can be converted to word→gloss for a larger glossary. Python library: [wn.readthedocs.io](https://wn.readthedocs.io/) — e.g. `wn.download("oewn:2025")` then iterate synsets for glosses.
- **Links**: [github.com/globalwordnet/english-wordnet](https://github.com/globalwordnet/english-wordnet), [en-word.net](https://en-word.net/).

### Wiktionary

- **What**: Collaborative dictionary; very large, multi-language.
- **License**: CC BY-SA.
- **How we use it**: `scripts/glossary_wiktionary.py` calls the English Wiktionary API to fill gaps for words missing from WordNet.
- **Links**: [en.wiktionary.org](https://en.wiktionary.org/), [dumps.wikimedia.org/enwiktionary](https://dumps.wikimedia.org/enwiktionary/). For bulk: Wiktextract or similar to parse dumps.

### LibreOffice dictionaries

- **What**: Hunspell .aff/.dic for many languages.
- **License**: MPL 2.0.
- **How we use it**: `scripts/update_dict_glossary.py --fetch-dict` downloads en_US.aff and en_US.dic to expand the spell-check word list. Then we run `build_glossary.py` so the **glossary** covers that expanded list (WordNet → Wiktionary → LLM).
- **Links**: [github.com/LibreOffice/dictionaries](https://github.com/LibreOffice/dictionaries).

### Other useful sources (no definitions)

- **dwyl/english-words**: Large word lists (e.g. words_alpha.txt) — useful as **input word lists** for building a glossary, not as a source of definitions.
- **GCIDE / Webster’s 1913**: Public-domain definitions; can be parsed for word→definition if you add a parser.

---

## Recommended approach for high coverage

1. **Use a full dictionary**: Fetch LibreOffice en_US (or another large Hunspell dict) so the spell checker knows many more words.
2. **Build the glossary from that word list**: Run `scripts/build_glossary.py` (with WordNet, then Wiktionary, then optional LLM) so most spelled words have at least one definition.
3. **Ship a pre-built glossary (optional)**: For the default install, you can commit a `data/glossary.txt` generated from the bundled or default dict so users get context clues without running Python.
4. **Confusables**: Prefer that commonly confused pairs (e.g. there/their, where/were) always have definitions; the test suite can require both words in each pair to be in the glossary.

---

## Test coverage of words

- **Overall coverage** (`[coverage]`): At least **80%** of words in the bundled `data/dict/en_US.dic` must have at least one entry in `data/glossary.txt`. See `tests/test_definition_coverage.cpp`.
- **Confusables** (`[coverage][confusables]`): A set of word pairs that are often confused (e.g. *there*/*their*, *where*/*were*). The test requires that **both** words in each pair have a glossary entry, so the UI can show definitions for both and help users choose by meaning.

After changing the dictionary or glossary, run:

```bash
./build/spell_tests "[coverage]"
```

See [GLOSSARY.md](GLOSSARY.md) for building and updating the glossary.
