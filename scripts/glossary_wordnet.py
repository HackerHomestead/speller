#!/usr/bin/env python3
"""
Export definitions from WordNet (NLTK) to glossary format:
  word<TAB>pos<TAB>short_definition

Usage:
  python glossary_wordnet.py [word_list.txt]
  If word_list.txt omitted, reads from stdin (one word per line).
  Writes to stdout. Skip words with no WordNet entry.
"""
from __future__ import annotations

import sys

try:
    from nltk.corpus import wordnet as wn
except ImportError:
    print("WordNet requires NLTK. Install: pip install nltk && python -c \"import nltk; nltk.download('wordnet'); nltk.download('omw-1.4')\"", file=sys.stderr)
    sys.exit(1)

POS_MAP = {
    wn.NOUN: "n.",
    wn.VERB: "v.",
    wn.ADJ: "adj.",
    wn.ADV: "adv.",
    wn.ADJ_SAT: "adj.",
}


def pos_tag(ss):
    return POS_MAP.get(ss.pos(), "")


def get_first_gloss(word: str) -> list[tuple[str, str, str]]:
    """Return list of (word, pos, gloss) for all synsets (one per sense)."""
    out = []
    seen = set()
    for synset in wn.synsets(word):
        pos = pos_tag(synset)
        gloss = (synset.definition() or "").strip()
        if not gloss:
            continue
        key = (word.lower(), pos, gloss)
        if key in seen:
            continue
        seen.add(key)
        out.append((word.lower(), pos, gloss))
    return out


def main():
    if len(sys.argv) > 1:
        with open(sys.argv[1], "r", encoding="utf-8") as f:
            lines = f.readlines()
    else:
        lines = sys.stdin.readlines()

    words = set()
    for line in lines:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if line.isdigit():
            continue
        w = line.split("/")[0].strip()
        if w:
            words.add(w.lower())

    for w in sorted(words):
        for word, pos, gloss in get_first_gloss(w):
            print(f"{word}\t{pos}\t{gloss}")


if __name__ == "__main__":
    main()
