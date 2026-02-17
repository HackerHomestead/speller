# Executive Summary

This document describes the design of a console‑based spell checker and lightweight definition tool intended for neurodivergent users (ADHD, dyslexia, dysgraphia) as a new module within the vibePDA project on an embedded Linux device. The solution is a single statically linked C/C++ binary that embeds a modular “service‑style” architecture without requiring separate daemons. It uses offline dictionaries for spell checking and simple definitions, and optionally supports multi‑algorithm suggestion pipelines that approximate “Did you mean…?” behavior familiar from web search.[9][10][11][12][13][14][15]

The key goals are:  
- Low‑friction, forgiving console UX tuned for neurodivergent users.  
- Offline‑first operation with small, manageable resource usage on constrained hardware.  
- Clean separation of concerns via internal service abstractions (spell engine, definition store, suggestion orchestrator, TUI controller) that integrate cleanly with vibePDA’s existing module system.

***

# 1. Introduction

## 1.1 Purpose

This Software Design Document describes the architecture and detailed design of the **vibePDA Spell & Definition Module** (“Spell Module”), a console utility for spell checking and simple word lookup integrated into vibePDA. It is intended for developers and maintainers of vibePDA as a reference for implementation, testing, and future enhancement.[10][9]

## 1.2 Scope

In scope:  
- Word and sentence‑level spell checking using offline dictionaries (Hunspell/Aspell or similar).[9][10]
- Ranked spelling suggestions using multiple algorithms and backends.  
- Short, simple definitions from an offline dataset (e.g., WordNet‑derived or custom minimal glossary).[16][17][18]
- Console UI modes: interactive and stream (stdin/file) modes.  
- Static linking and embedded‑friendly packaging on Linux.[11][12][13]

Out of scope (initial version):  
- Full grammar checking.  
- Rich natural‑language understanding, deep context models.  
- Official online “Google” integration (may be considered later via generic web suggestion APIs).[19]

***

# 2. System Overview

## 2.1 High‑Level Description

The Spell Module is a new sub‑command within vibePDA (for example, `vibe spell`) compiled into the same executable as other modules. Internally, it is organized as a set of C++ components that behave like services within a single process:[13]

- **SpellEngine**: wraps one or more spell‑checking libraries (e.g., Hunspell or Aspell) for correctness checks and base suggestions.[10][9]
- **DefinitionStore**: provides short definitions from a compact on‑disk database.  
- **SuggestionOrchestrator**: merges and ranks suggestions from multiple algorithms.  
- **SpellTui**: console UI tailored for neurodivergent users.

The module reads and writes plain text (stdin/stdout) and shares vibePDA’s logging, configuration, and build system.

## 2.2 Technology Stack

- Language: C/C++ (C++ for high‑level orchestration and interfaces; C APIs where required).  
- Spell libraries: Hunspell and/or GNU Aspell.[9][10]
- Definition  local files derived from WordNet or a custom minimal dictionary.[17][16]
- Platform: Linux (embedded), static or mostly‑static linking.[13]
- Terminal: standard ANSI terminal, optionally `ncurses` if needed.

***

# 3. Requirements

## 3.1 Functional Requirements

1. **Offline spell checking**  
   - The system shall check individual words and lines of text against an offline dictionary.[20][10][9]
2. **Suggestion generation**  
   - For misspelled words, the system shall generate a ranked list of candidate corrections (“Did you mean…?”).[14][15][10][9]
3. **Definitions**  
   - On user request (e.g., pressing a key), the system shall display a short definition for a selected word.[16][17]
4. **Interactive mode**  
   - The system shall allow stepping through text word‑by‑word with single‑key actions: choose suggestion, skip, undo, show definition.  
5. **Stream mode**  
   - The system shall accept text from stdin or a file and output corrected text to stdout, optionally with minimal user interaction.  
6. **User dictionary**  
   - The system shall support a user‑maintained dictionary for custom words, stored locally and applied to spell checks.[21][22]
7. **Integration with vibePDA**  
   - The system shall be accessible as a sub‑command or module from the existing vibePDA entry point.

## 3.2 Non‑Functional Requirements

- **Performance**:  
  - Spell checking should keep up with typical typing/streaming speeds on the target hardware (e.g., correcting a typical paragraph in under a second).  
- **Resource usage**:  
  - Executable plus dictionaries and definitions should fit within the device’s flash/storage budget (e.g., tens of MB, depending on selected language and dictionary size).[12][11][17]
- **Reliability**:  
  - Operation must be robust to missing data files (graceful degradation: spell checking without definitions, etc.).  
- **Usability (Neurodivergent focus)**:  
  - Minimize cognitive load with simple layouts, short definitions by default, and minimal keystrokes for common actions.  
- **Maintainability**:  
  - The module should follow vibePDA’s coding and module patterns, with clear internal interfaces and minimal coupling.

***

# 4. Architecture

## 4.1 High‑Level Architecture

Conceptually, the Spell Module fits into vibePDA as an additional mode:

- **vibePDA main**  
  - Parses CLI arguments.  
  - Selects module based on sub‑command (`spell`, etc.).  
  - Passes configuration and I/O streams to the selected module.

Within the Spell Module:

- **SpellEngine** (interface + implementations)  
  - Uses Hunspell and/or Aspell to implement spell checking and base suggestions.[23][21][10][9]
- **DefinitionStore**  
  - Uses a small on‑disk store with word → short gloss mappings (e.g., WordNet subset).[17][16]
- **SuggestionOrchestrator**  
  - Combines multiple candidate sources and ranks them using frequency and edit distance.[15]
- **SpellTui**  
  - Provides interaction loops for interactive and stream modes.

All components operate in‑process; no external daemons, sockets, or IPC are required.

## 4.2 Component Overview

- `SpellEngine`  
  - Abstract base class with `is_correct()` and `suggest()` methods.  
  - Implementations: `HunspellEngine`, `AspellEngine`, and potentially others.  
- `DefinitionStore`  
  - Abstract interface for `lookup(word)` returning a small `Definition` struct.  
  - Implementation(s): `MinimalDefinitionStore` using a custom binary or SQLite‑like file; optional WordNet‑backed variant.[16][17]
- `SuggestionOrchestrator`  
  - Accepts one or more `SpellEngine` instances.  
  - Generates, merges, deduplicates, and scores suggestions.  
- `SpellTui`  
  - Handles user interaction via keyboard, draws suggestions and definitions.  
  - Provides both line/sentence and word‑by‑word flows.

***

# 5. Detailed Design

## 5.1 Module Interfaces

### 5.1.1 SpellEngine Interface

```cpp
struct Suggestion {
    std::string word;
    float       score;    // Higher = better
};

class SpellEngine {
public:
    virtual ~SpellEngine() = default;

    virtual bool is_correct(const std::string& word) = 0;

    virtual std::vector<Suggestion> suggest(const std::string& word) = 0;
};
```

### 5.1.2 HunspellEngine

Implements `SpellEngine` using Hunspell’s C++ API.[9]

Key responsibilities:  
- Load dictionaries (`.aff`, `.dic`) from configured paths.  
- Forward spelling queries and suggestions to Hunspell.  
- Map Hunspell’s suggestion list to `Suggestion` objects with base scores.

### 5.1.3 AspellEngine (Optional)

Implements `SpellEngine` using ASPell’s C API.[21][23]

Useful if:  
- You want Aspell’s learning/replacement mechanisms and user dictionary features.  
- You want redundancy across engines.

### 5.1.4 DefinitionStore Interface

```cpp
struct Definition {
    std::string headword;
    std::string part_of_speech;
    std::string short_gloss;

    bool empty() const { return headword.empty(); }
};

class DefinitionStore {
public:
    virtual ~DefinitionStore() = default;
    virtual Definition lookup(const std::string& word) = 0;
};
```

Implementation options:  
- **Compact custom DB**: custom binary file mapping words to short glosses; small and fast.  
- **WordNet‑derived DB**: preprocessed subset of WordNet databases filtered to common words and one or two senses.[18][17][16]

## 5.2 SuggestionOrchestrator

```cpp
class SuggestionOrchestrator {
public:
    SuggestionOrchestrator(std::vector<std::unique_ptr<SpellEngine>> engines);

    std::vector<Suggestion> suggest(const std::string& word);

private:
    std::vector<std::unique_ptr<SpellEngine>> engines_;
    float frequency_score(const std::string& word);
    float edit_distance_penalty(const std::string& input,
                                const std::string& candidate);
};
```

Behavior:

1. For a misspelled word, call `suggest()` on each engine.  
2. Merge results into a map `candidate → accumulated score`.  
3. Apply additional scoring based on:  
   - Word frequency (from a local frequency list or corpus).  
   - Edit distance between input and candidate (Levenshtein distance).[15]
4. Sort and return top N suggestions (e.g., 5).

This provides a multi‑algorithm, multi‑backend “Did you mean…?” effect while remaining offline.

## 5.3 User Interface (SpellTui)

### 5.3.1 Design Goals

- Minimize cognitive load and decision fatigue.  
- Provide predictable, simple keystrokes.  
- Keep layout uncluttered and high‑contrast.

### 5.3.2 Key Flows

**Interactive “Word‑By‑Word” Mode**

For each word:

1. If correct: show word lightly, move on or silently skip in “fast mode”.  
2. If misspelled:  
   - Highlight the word clearly.  
   - Show top 3–5 suggestions numbered (1–5).  
   - Provide keys:  
     - `1–5`: pick suggestion.  
     - `s`: skip word.  
     - `u`: undo previous change.  
     - `d`: show definition of selected suggestion or the original word.  
     - `q`: quit, optionally writing a partial result.

**Stream Mode**

- Input: from stdin or file.  
- User options:  
  - Automatic apply of top suggestion.  
  - Prompt only when engine confidence is low.  
- Output: corrected text to stdout, optionally with markers or logs.

### 5.3.3 Accessibility Considerations

- Allow configurable verbosity: brief vs. detailed mode.  
- Keep definitions short by default; an extra key reveals longer glosses.  
- Minimize text density: clearly separated lines, minimal decoration.  
- Consider options for line spacing and color/brightness that are friendly for dyslexic users.

## 5.4 Data Structures and Storage

- **Dictionary files**: Hunspell/Aspell `.dic` and `.aff` files stored under a known directory (e.g., `/opt/vibe/dict/en_US.*`).[11][12][20][9]
- **User dictionary**: small text or binary file with words added/learned interactively (e.g., `user.dic`).  
- **Definition DB**: custom binary or simple key‑value file containing headword, POS, and short gloss; built offline from WordNet or similar public resources.[17][16]
- **Frequency list**: simple text or binary file of `word → frequency`, used only for scoring and entirely local.

***

# 6. Integration with vibePDA

## 6.1 Command‑Line Integration

- New sub‑command: `vibe spell [options]`.  
- Example options:  
  - `--interactive` (default).  
  - `--file path/to/file`.  
  - `--dict-dir path/to/dict`.  
  - `--defs path/to/defs.db`.  
  - `--fast` vs. `--careful`.

`main()` dispatches to a `run_spell(const Config&)` function, following the same pattern as other vibePDA tools.

## 6.2 Build and Packaging

- Build the spell module as part of the main vibePDA build:  
  - `src/spell/` and `include/vibe/spell/`.  
  - Statically link spell libraries where feasible.[12][11][13]
- Install data files alongside other vibePDA assets:  
  - `/opt/vibe/dict/` for dictionaries.  
  - `/opt/vibe/defs/` for definition DB.  
- Use vibePDA’s existing configuration system for paths and options.

***

# 7. Deployment and Resource Considerations

## 7.1 Static Linking

- Rationale:  
  - Embedded Linux devices often have limited or custom userlands; static linking avoids missing shared‑library dependencies.[13]
- Approach:  
  - Link Hunspell/Aspell statically where licensing/toolchain allow.  
  - Use either musl or glibc according to the existing vibePDA toolchain.[24][13]

## 7.2 Storage Footprint

Approximate orders of magnitude (single language):  

- Hunspell core library: ~1 MB installed size.[11][12]
- Hunspell dictionary + affix for English: ~1 MB.[20]
- Minimal definition DB: configurable; a subset derived from WordNet can be trimmed to tens of MB or less depending on coverage.[18][17]

Strategies to keep size down:

- Ship only one language initially.  
- Trim definition DB to most common words plus user‑relevant vocabulary.  
- Use compressed or compact binary formats for definitions and frequency lists.

## 7.3 Performance

- Load dictionaries once at startup; reuse engine instances across the session.  
- Optionally memory‑map definition DB for fast lookups.  
- Keep edit‑distance and scoring algorithms lightweight.

***

# 8. Implementation Plan

## 8.1 Phases

1. **Phase 1 – Core Skeleton (1–2 weeks)**  
   - Define `SpellEngine`, `DefinitionStore`, `SuggestionOrchestrator`, `SpellTui` interfaces.  
   - Integrate a minimal stub module into vibePDA (`vibe spell` prints a placeholder).

2. **Phase 2 – Spell Engine Integration (2–3 weeks)**  
   - Integrate Hunspell or Aspell as a `SpellEngine` implementation.[23][10][21][9]
   - Wire dictionary loading and basic `is_correct()` / `suggest()` behavior.  
   - Add basic user dictionary handling.

3. **Phase 3 – Suggestion Orchestration and Scoring (2 weeks)**  
   - Implement `SuggestionOrchestrator` with frequency‑ and edit‑distance‑based ranking.[15]
   - Add configuration options for number of suggestions and scoring parameters.  
   - Add tests for common typo patterns.

4. **Phase 4 – Definition Store (2–3 weeks)**  
   - Design and generate a minimal definition DB from WordNet or another source (done off‑device).[18][16][17]
   - Implement `DefinitionStore` lookup.  
   - Integrate definition display in the TUI (`d` key).

5. **Phase 5 – TUI and Accessibility (2–3 weeks)**  
   - Implement interactive and stream modes.  
   - Tune layout, wording, and keybindings for ADHD/dyslexic/dysgraphic users.  
   - Add configuration flags for verbosity, colors, spacing.

6. **Phase 6 – Optimization and Embedded Tuning (1–2 weeks)**  
   - Measure memory and CPU usage on target hardware.  
   - Optimize build (LTO, dead‑code elimination) and data (trim dictionaries/definitions).

7. **Phase 7 – Documentation and Release (1 week)**  
   - Document module usage, configuration, and integration points.  
   - Prepare developer documentation for future contributors.

## 8.2 Risks and Mitigations

- **Storage constraints**:  
  - Risk: dictionaries and definition sets are too large.  
  - Mitigation: start with core language and minimal glosses; allow disabling definitions on very small devices.

- **Library licensing / static linking issues**:  
  - Risk: incompatibilities between static linking, toolchain, or licenses.  
  - Mitigation: evaluate both Hunspell and Aspell, use whichever integrates cleanly.

- **Usability for neurodivergent users**:  
  - Risk: UI becomes too complex or overwhelming.  
  - Mitigation: user testing, simple defaults, and adjustable verbosity modes.

***

# 9. Conclusion

This design enables a robust, offline‑first spell checker and definition tool integrated into vibePDA, optimized for embedded Linux and neurodivergent users. The modular, service‑style architecture within a single statically linked binary offers both maintainability and deployment simplicity, while the multi‑algorithm suggestion pipeline and local definitions deliver a familiar yet resource‑friendly “Did you mean…?” experience.[14][10][13][15][17][9]

***

To create the PDF, you can paste this into your documentation tool of choice (Markdown, LaTeX, word processor) and export as PDF; if you share your preferred format, I can adapt it to that style directly.

Sources
[1] Technical Design Document Template | PDF | Data Model - Scribd https://www.scribd.com/document/320454748/10-Technical-Design-Document-Template
[2] [PDF] Software Design Document (SDD) Template (summarized from IEEE https://wildart.github.io/MISG5020/standards/SDD_Template.pdf
[3] [DOC] Technical Design Document.docx - Stanford University https://uit.stanford.edu/sites/default/files/2017/08/30/Technical%20Design%20Document.docx
[4] [PDF] Technical Design Document - Katoliski institut https://www.katoliski-institut.si/public_resources_cache/files/original/2/287/d_t4_2_2_technical_documentation_the_for_web_based_tool_final_version.pdf
[5] [PDF] Design Document https://www.se.rit.edu/~sis-io/documents/DesignDocument_updated.pdf
[6] [DOC] Technical Design Document (TDD) - CaPPMS https://umgc-cappms.azurewebsites.net/download/943176d4-4660-4c0a-9ddf-1c96fa8b264a----Technical%20Design%20Document_Mobile%20Team.docx
[7] [DOC] https://www.cms.gov/files/zip/highleveltechnicalde... https://www.cms.gov/files/zip/highleveltechnicaldesignzip
[8] [PDF] Software Design Document - Bellevue College https://www.bellevuecollege.edu/wp-content/uploads/sites/135/2019/04/SDD_RoadTrip.pdf
[9] hunspell/hunspell: The most popular spellchecking library. - GitHub https://github.com/hunspell/hunspell
[10] GNU Aspell http://aspell.net
[11] Debian -- Details of package hunspell in sid https://packages.debian.org/sid/hunspell
[12] hunspell 1.7.2-2 (x86_64) - Arch Linux https://archlinux.org/packages/extra/x86_64/hunspell/
[13] How static linking works on Linux | Opensource.com https://opensource.com/article/22/6/static-linking-linux
[14] How does the Google "Did you mean?" Algorithm work? [closed] https://stackoverflow.com/questions/307291/how-does-the-google-did-you-mean-algorithm-work
[15] Mind your words with NLP https://blog.chatbotslife.com/mind-your-words-with-nlp-f352670af765
[16] wnintro(3WN) - WordNet - Princeton University https://wordnet.princeton.edu/documentation/wnintro3wn
[17] readme https://wordnetcode.princeton.edu/1.7/README
[18] readme https://wordnetcode.princeton.edu/README
[19] Google Spell Check API - SerpApi https://serpapi.com/spell-check
[20] Debian -- Details of package hunspell-en-gb in bookworm https://packages.debian.org/bookworm/hunspell-en-gb
[21] 6. Writing programs to use Aspell http://aspell.net/0.50-doc/man-html/6_Writing.html
[22] Creating custom dictionaries in aspell with the C API - Stack Overflow https://stackoverflow.com/questions/1292241/creating-custom-dictionaries-in-aspell-with-the-c-api
[23] 6.1 Through the C API - GNU Aspell http://aspell.net/man-html/Through-the-C-API.html
[24] How can I build a statically linked nu for x86_64-unknown-linux-musl? https://github.com/nushell/nushell/discussions/13516

