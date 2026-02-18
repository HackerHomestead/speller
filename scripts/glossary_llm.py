#!/usr/bin/env python3
"""
Generate glossary lines (word<TAB>pos<TAB>definition) using an LLM for words that
have no definition yet. Supports Ollama (local) or OpenAI-compatible API.

Usage:
  python glossary_llm.py [word_list.txt]
  Reads word_list.txt or stdin (one word per line). Writes to stdout.

Environment:
  OLLAMA_MODEL   - e.g. mistral (default). If set, uses Ollama.
  OPENAI_API_KEY - If set (and no OLLAMA_MODEL), uses OpenAI API.
  OPENAI_BASE_URL - Optional base URL for OpenAI-compatible API.
"""
from __future__ import annotations

import json
import os
import subprocess
import sys

try:
    import requests
except ImportError:
    requests = None


def words_from_input(path: str | None) -> set[str]:
    if path:
        with open(path, "r", encoding="utf-8") as f:
            lines = f.readlines()
    else:
        lines = sys.stdin.readlines()
    out = set()
    for line in lines:
        line = line.strip()
        if not line or line.startswith("#") or line.isdigit():
            continue
        w = line.split("/")[0].strip().lower()
        if w:
            out.add(w)
    return out


def call_ollama(words: list[str], model: str) -> list[tuple[str, str, str]]:
    prompt = """For each word below, output exactly one line in this format: word<TAB>part_of_speech<TAB>short definition (one phrase, no newlines).
Use pos like n. v. adj. adv. prep. conj. interj. det. art. pron.
Words (one per line):
"""
    prompt += "\n".join(words[:50])
    try:
        r = subprocess.run(
            ["ollama", "run", model, prompt],
            capture_output=True,
            text=True,
            timeout=120,
        )
        out = []
        for line in r.stdout.splitlines():
            line = line.strip()
            if "\t" in line and not line.startswith("#"):
                parts = line.split("\t", 2)
                if len(parts) >= 3:
                    w, pos, gloss = parts[0].strip().lower(), parts[1].strip(), parts[2].strip()
                    if w and gloss:
                        out.append((w, pos, gloss))
        return out
    except Exception as e:
        print(f"# ollama error: {e}", file=sys.stderr)
        return []


def call_openai(words: list[str]) -> list[tuple[str, str, str]]:
    if not requests:
        print("OpenAI path requires: pip install requests", file=sys.stderr)
        return []
    key = os.environ.get("OPENAI_API_KEY")
    base = os.environ.get("OPENAI_BASE_URL", "https://api.openai.com/v1")
    if not key:
        return []
    prompt = """For each word below, output exactly one line: word<TAB>pos<TAB>short definition (one phrase).
Use pos: n. v. adj. adv. prep. conj. interj. det. art. pron.
Words:
"""
    prompt += ", ".join(words[:30])
    try:
        r = requests.post(
            f"{base.rstrip('/')}/chat/completions",
            headers={"Authorization": f"Bearer {key}", "Content-Type": "application/json"},
            json={
                "model": os.environ.get("OPENAI_MODEL", "gpt-4o-mini"),
                "messages": [{"role": "user", "content": prompt}],
                "temperature": 0.3,
            },
            timeout=60,
        )
        r.raise_for_status()
        text = r.json()["choices"][0]["message"]["content"]
        out = []
        for line in text.splitlines():
            line = line.strip()
            if "\t" in line and not line.startswith("#"):
                parts = line.split("\t", 2)
                if len(parts) >= 3:
                    w, pos, gloss = parts[0].strip().lower(), parts[1].strip(), parts[2].strip()
                    if w and gloss:
                        out.append((w, pos, gloss))
        return out
    except Exception as e:
        print(f"# openai error: {e}", file=sys.stderr)
        return []


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else None
    words = sorted(words_from_input(path))
    if not words:
        return

    model = os.environ.get("OLLAMA_MODEL", "mistral")
    if os.environ.get("OLLAMA_MODEL") or subprocess.run(["which", "ollama"], capture_output=True).returncode == 0:
        for i in range(0, len(words), 20):
            batch = words[i : i + 20]
            for w, pos, gloss in call_ollama(batch, model):
                print(f"{w}\t{pos}\t{gloss}")
    elif os.environ.get("OPENAI_API_KEY") and requests:
        for i in range(0, len(words), 30):
            batch = words[i : i + 30]
            for w, pos, gloss in call_openai(batch):
                print(f"{w}\t{pos}\t{gloss}")
    else:
        print("Set OLLAMA_MODEL and have 'ollama' installed, or set OPENAI_API_KEY. See script doc.", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
