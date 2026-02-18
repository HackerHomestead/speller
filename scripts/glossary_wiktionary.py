#!/usr/bin/env python3
"""
Fetch definitions from English Wiktionary API for a list of words.
Output: word<TAB>pos<TAB>short_definition (one per line).

Usage:
  python glossary_wiktionary.py [word_list.txt]
  Reads word_list.txt or stdin (one word per line). Writes to stdout.
  Requires: pip install requests
"""
from __future__ import annotations

import re
import sys
import time

try:
    import requests
except ImportError:
    print("Wiktionary API requires requests. Install: pip install requests", file=sys.stderr)
    sys.exit(1)

WIKTIONARY_API = "https://en.wiktionary.org/w/api.php"
RATE_DELAY = 0.2


def pos_short(tag: str) -> str:
    if not tag:
        return ""
    t = tag.lower()
    if t in ("noun", "n"): return "n."
    if t in ("verb", "v"): return "v."
    if t in ("adjective", "adj"): return "adj."
    if t in ("adverb", "adv"): return "adv."
    if t in ("preposition", "prep"): return "prep."
    if t in ("conjunction", "conj"): return "conj."
    if t in ("pronoun", "pron"): return "pron."
    if t in ("interjection", "interj"): return "interj."
    if t in ("determiner", "det"): return "det."
    if t in ("article", "art"): return "art."
    return tag + "."


def strip_wiki_markup(text: str) -> str:
    text = re.sub(r"\[\[([^\]|]+\|[^\]]+)\]\]", r"\1", text)
    text = re.sub(r"\[\[([^\]]+)\]\]", r"\1", text)
    text = re.sub(r"{{[^}]+}}", "", text)
    text = re.sub(r"<[^>]+>", "", text)
    text = text.replace("'''", "").strip()
    return text[:200].strip()


def fetch_definitions(word: str) -> list[tuple[str, str, str]]:
    out = []
    params = {
        "action": "query",
        "titles": word,
        "prop": "revisions",
        "rvprop": "content",
        "rvslots": "main",
        "format": "json",
        "formatversion": "2",
    }
    try:
        r = requests.get(WIKTIONARY_API, params=params, timeout=10)
        r.raise_for_status()
        data = r.json()
    except Exception as e:
        if __name__ == "__main__":
            print(f"# wiktionary error for '{word}': {e}", file=sys.stderr)
        return out

    pages = data.get("query", {}).get("pages", [])
    if not pages or "missing" in pages[0]:
        return out

    content = pages[0].get("revisions", [{}])[0].get("slots", {}).get("main", {}).get("content", "")
    if not content:
        return out

    # Simple extraction: look for ===English=== and then ====Noun==== etc. and definition lines
    in_english = False
    current_pos = ""
    for line in content.split("\n"):
        line = line.strip()
        if line.startswith("===English==="):
            in_english = True
            continue
        if in_english and line.startswith("===") and not line.startswith("===="):
            break
        if line.startswith("====") and line.endswith("===="):
            current_pos = line.strip("=").strip()
            continue
        if current_pos and line.startswith("# ") and not line.startswith("#:"):
            defn = line.lstrip("# ").strip()
            defn = strip_wiki_markup(defn)
            if defn and len(defn) > 2:
                pos = pos_short(current_pos)
                out.append((word.lower(), pos, defn))
                break
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
        if not line or line.startswith("#") or line.isdigit():
            continue
        w = line.split("/")[0].strip().lower()
        if w:
            words.add(w)

    for w in sorted(words):
        for word, pos, gloss in fetch_definitions(w):
            print(f"{word}\t{pos}\t{gloss}")
        time.sleep(RATE_DELAY)


if __name__ == "__main__":
    main()
