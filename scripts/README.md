# Scripts

Optional Python scripts for building and updating the glossary and dictionary.

## Requirements

```bash
pip install -r scripts/requirements-glossary.txt
python -c "import nltk; nltk.download('wordnet'); nltk.download('omw-1.4')"
```

## Glossary build

| Script | Purpose |
|--------|--------|
| **build_glossary.py** | Build `data/glossary.txt` from a word list. Pipeline: WordNet → Wiktionary (gaps) → LLM (gaps). |
| **glossary_wordnet.py** | Export WordNet (NLTK) definitions only. |
| **glossary_wiktionary.py** | Export Wiktionary API definitions only. |
| **glossary_llm.py** | Export LLM-generated definitions (Ollama or OpenAI). |

See **[../docs/GLOSSARY.md](../docs/GLOSSARY.md)** for full documentation.

### Examples

```bash
# WordNet only (fast, no API)
python scripts/build_glossary.py --no-wiktionary

# WordNet + Wiktionary (default; LLM is off by default)
python scripts/build_glossary.py

# With LLM gap-fill (set OLLAMA_MODEL or OPENAI_API_KEY)
python scripts/build_glossary.py --llm
```

## Dictionary and glossary update

| Script | Purpose |
|--------|--------|
| **update_dict_glossary.py** | Optionally download en_US from LibreOffice dicts, then run build_glossary. |

```bash
# Rebuild glossary from existing data/dict/en_US.dic
python scripts/update_dict_glossary.py

# Download new dictionary and rebuild glossary
python scripts/update_dict_glossary.py --fetch-dict
```
