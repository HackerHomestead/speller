#include "spell/repl.hpp"
#include "spell/file_definition_store.hpp"
#include "spell/hunspell_engine.hpp"
#include "spell/stub_definition_store.hpp"
#include "spell/definition_store.hpp"
#include "spell/tokenize.hpp"
#include "util/affirmations.hpp"
#include "util/dict_path.hpp"
#include "util/term_format.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <memory>
#include <sstream>
#include <vector>

#ifdef SPELL_HAS_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#endif

namespace spell {

namespace {

const char* repl_help =
    "\n  spell REPL - Interactive spell checker\n"
    "  --------------------------------------\n\n"
    "  Type a word or sentence and press Enter to check spelling.\n"
    "  Commands (start with : or use full name):\n\n"
    "    help, ?        Show this help\n"
    "    load PATH      Load dictionary (PATH = dir or path to .aff file)\n"
    "    :load PATH     Same as load\n"
    "    dict           Show current dictionary path\n"
    "    define WORD    Show definition for WORD (from glossary)\n"
    "    def WORD       Same as define (Tab cycles through last \"Did you mean\" suggestions)\n"
    "    correct TEXT   Correct a sentence (interactive word-by-word)\n"
    "    rework TEXT    Rework mode: retype sentence to build muscle memory\n"
    "    quit, exit     Exit the REPL\n"
    "    :q             Shortcut to quit\n\n"
    "  Examples:\n\n"
    "    spell> hello\n"
    "    Correct!\n\n"
    "    spell> helo\n"
    "    Did you mean: hello?\n\n"
    "    spell> correct I haev a probelm\n"
    "    (shows misspelled words with suggestions, prompts for correction)\n\n"
    "    spell> rework I haev a probelm\n"
    "    (retype sentence, misspelled words shown in red, Tab for suggestions)\n\n"
    "    spell> load /usr/share/hunspell\n"
    "    Loaded dictionary: /usr/share/hunspell (en_US)\n\n"
    "    spell> load ./data/dict/en_US.aff\n"
    "    Loaded dictionary: ./data/dict (en_US)\n\n"
    "    spell> help\n"
    "    (shows this help)\n\n"
    "    spell> :q\n"
    "    (exits)\n\n"
    "  Tips:\n"
    "  - Tab completes commands (help, load, quit, :q)\n"
    "  - Up/Down for command history\n"
    "  - Default: bundled en_US dictionary\n"
    "  - Empty line does nothing\n\n";

const char* prompt = "spell> ";

#ifdef SPELL_HAS_READLINE
const char* repl_commands[] = {"help", "?", "load", ":load", "dict", "define", "def", "correct", "rework", "quit", "exit", ":q", nullptr};

// Last "Did you mean" suggestion words, for Tab-completing "def WORD" / "define WORD"
static std::vector<std::string> last_suggestion_words;

char* repl_command_generator(const char* text, int state) {
  static size_t i;
  if (state == 0) i = 0;
  while (repl_commands[i]) {
    std::string cmd(repl_commands[i++]);
    if (cmd.find(text) == 0) {
      return strdup(cmd.c_str());
    }
  }
  return nullptr;
}

// Complete from last_suggestion_words: filter by prefix, return one per state (cycle).
char* repl_suggestion_word_generator(const char* text, int state) {
  static size_t index;
  if (state == 0) index = 0;
  std::string prefix(text);
  while (index < last_suggestion_words.size()) {
    const std::string& word = last_suggestion_words[index++];
    if (word.size() >= prefix.size() &&
        word.compare(0, prefix.size(), prefix) == 0) {
      return strdup(word.c_str());
    }
  }
  return nullptr;
}

char** repl_completion(const char* text, int start, int end) {
  (void)end;
  if (start == 0) {
    return rl_completion_matches(text, repl_command_generator);
  }
  // After "def " or "define ", complete from last suggestion list (cycle with Tab)
  const char* line = rl_line_buffer;
  if (line && !last_suggestion_words.empty()) {
    std::string s(line);
    if ((s.size() >= 4 && s.substr(0, 4) == "def ") ||
        (s.size() >= 8 && s.substr(0, 8) == "define ")) {
      return rl_completion_matches(text, repl_suggestion_word_generator);
    }
  }
  return nullptr;
}
#endif

std::string trim(const std::string& s) {
  auto start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) return "";
  auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

std::string to_lower(const std::string& s) {
  std::string r = s;
  for (char& c : r) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return r;
}

bool is_quit_command(const std::string& line) {
  std::string lower = to_lower(trim(line));
  return lower == "quit" || lower == "exit" || lower == ":q" || lower == "q";
}

bool is_help_command(const std::string& line) {
  std::string lower = to_lower(trim(line));
  return lower == "help" || lower == "?" || lower == ":help";
}

bool is_load_command(const std::string& line, std::string& out_path) {
  std::string t = trim(line);
  if (t.empty()) return false;
  std::string lower = to_lower(t);
  if (lower.size() >= 5 && lower.substr(0, 5) == "load ") {
    out_path = trim(t.substr(5));
    return !out_path.empty();
  }
  if (lower.size() >= 6 && lower.substr(0, 6) == ":load ") {
    out_path = trim(t.substr(6));
    return !out_path.empty();
  }
  return false;
}

bool is_dict_command(const std::string& line) {
  return to_lower(trim(line)) == "dict";
}

bool is_define_command(const std::string& line, std::string& out_word) {
  std::string t = trim(line);
  if (t.empty()) return false;
  std::string lower = to_lower(t);
  if (lower.size() > 7 && lower.substr(0, 7) == "define ") {
    out_word = trim(t.substr(7));
    return !out_word.empty();
  }
  if (lower.size() > 4 && lower.substr(0, 4) == "def ") {
    out_word = trim(t.substr(4));
    return !out_word.empty();
  }
  return false;
}

std::string extract_word(const std::string& line) {
  std::string t = trim(line);
  if (t.empty()) return "";
  if (t[0] == ':') return "";
  std::string lower = to_lower(t);
  if (lower == "help" || lower == "?" || lower == "quit" || lower == "exit" || lower == "q") return "";
  if (lower == "dict") return "";
  if (lower.size() >= 7 && lower.substr(0, 7) == "define ") return "";
  if (lower.size() >= 4 && lower.substr(0, 4) == "def ") return "";
  if (lower.size() >= 5 && lower.substr(0, 5) == "load ") return "";
  if (lower.size() >= 6 && lower.substr(0, 6) == ":load ") return "";
  if (lower.size() >= 5 && lower.substr(0, 5) == "check") {
    return trim(t.substr(5));
  }
  if (lower.size() >= 8 && lower.substr(0, 8) == "correct ") {
    return "";
  }
  return t;
}

std::string to_lower_word(const std::string& s) {
  return to_lower(s);
}

void correct_sentence(const std::string& sentence, 
                      spell::SpellEngine* engine,
                      spell::DefinitionStore* defs) {
  std::vector<Token> tokens = tokenize_sentence(sentence);
  
  if (tokens.empty()) {
    std::cout << "No text to correct.\n";
    return;
  }
  
  std::cout << "\nOriginal: \"" << sentence << "\"\n\n";
  
  std::vector<size_t> misspelled_indices;
  std::vector<std::vector<spell::Suggestion>> all_suggestions;
  
  for (size_t i = 0; i < tokens.size(); ++i) {
    auto& token = tokens[i];
    if (!token.is_word) {
      std::cout << token.original;
      continue;
    }
    
    std::string lower = to_lower_word(token.text);
    
    if (engine->is_correct(lower)) {
      std::cout << token.original;
    } else {
      auto suggestions = engine->suggest(lower);
      std::cout << "[";
      term_bold_red(std::cout, token.original);
      std::cout << ":";
      if (suggestions.empty()) {
        std::cout << "?]";
      } else {
        size_t show_count = std::min(suggestions.size(), size_t(5));
        for (size_t j = 0; j < show_count; ++j) {
          if (j > 0) std::cout << ", ";
          std::cout << (j + 1) << ":";
          if (!defs->lookup(suggestions[j].word).empty())
            term_bold_yellow(std::cout, suggestions[j].word);
          else
            term_bold(std::cout, suggestions[j].word);
        }
        std::cout << "]";
        misspelled_indices.push_back(i);
        all_suggestions.push_back(suggestions);
      }
    }
  }
  std::cout << "\n";
  
  if (misspelled_indices.empty()) {
    std::cout << "All words are spelled correctly!\n";
    return;
  }
  
  std::cout << "\nCorrecting " << misspelled_indices.size() << " misspelled word(s)...\n";
  std::cout << "Enter number to apply to current word, 'a' for all remaining, 'n' for next, 'k' to keep original, 'q' to quit.\n\n";
  
  size_t current_idx = 0;
  int last_choice = -1;
  
  while (current_idx < misspelled_indices.size()) {
    size_t token_idx = misspelled_indices[current_idx];
    auto& token = tokens[token_idx];
    auto& suggestions = all_suggestions[current_idx];
    
    std::cout << "Word " << (current_idx + 1) << "/" << misspelled_indices.size() << ": ";
    term_bold_red(std::cout, token.original);
    if (!suggestions.empty()) {
      std::cout << " [";
      size_t show_count = std::min(suggestions.size(), size_t(5));
      for (size_t j = 0; j < show_count; ++j) {
        if (j > 0) std::cout << ", ";
        std::cout << (j + 1) << ":";
        if (!defs->lookup(suggestions[j].word).empty())
          term_bold_yellow(std::cout, suggestions[j].word);
        else
          term_bold(std::cout, suggestions[j].word);
      }
      std::cout << "]";
    } else {
      std::cout << " [no suggestions]";
    }
    std::cout << ": ";
    std::cout << std::flush;
    
    std::string response;
    if (!std::getline(std::cin, response)) return;
    
    response = trim(response);
    
    if (response == "q" || response == "quit") {
      std::cout << "Cancelled.\n";
      return;
    }
    
    if (response == "k" || response == "keep") {
      current_idx++;
      continue;
    }
    
    if (response == "n" || response == "next") {
      current_idx++;
      continue;
    }
    
    if (response == "a" || response == "all") {
      if (last_choice < 0) {
        std::cout << "  Choose a number first, then 'a' to apply to remaining.\n";
        continue;
      }
      size_t choice_idx = static_cast<size_t>(last_choice);
      while (current_idx < misspelled_indices.size()) {
        size_t ti = misspelled_indices[current_idx];
        auto& suggs = all_suggestions[current_idx];
        if (choice_idx < suggs.size()) {
          tokens[ti].text = suggs[choice_idx].word;
          if (!tokens[ti].original.empty() && 
              tokens[ti].original[0] >= 'A' && 
              tokens[ti].original[0] <= 'Z') {
            tokens[ti].text[0] = static_cast<char>(std::toupper(tokens[ti].text[0]));
          }
        }
        current_idx++;
      }
      std::cout << "  Applied choice " << (last_choice + 1) << " to remaining words.\n";
      break;
    }
    
    int choice = -1;
    try {
      choice = std::stoi(response);
    } catch (...) {
      std::cout << "  Invalid. Enter number (1-5), 'n' next, 'k' keep, 'q' quit.\n";
      continue;
    }
    
    if (choice < 1 || choice > 5) {
      std::cout << "  Invalid choice. Enter 1-5.\n";
      continue;
    }
    
    size_t choice_idx = static_cast<size_t>(choice - 1);
    if (choice_idx >= suggestions.size()) {
      std::cout << "  Not that many suggestions. Try 1-" << suggestions.size() << ".\n";
      continue;
    }
    
    token.text = suggestions[choice_idx].word;
    if (!token.original.empty() && 
        token.original[0] >= 'A' && 
        token.original[0] <= 'Z') {
      token.text[0] = static_cast<char>(std::toupper(token.text[0]));
    }
    std::cout << "  Applied: " << token.text << "\n";
    last_choice = static_cast<int>(choice_idx);
    current_idx++;
  }
  
  std::cout << "\nCorrected: \"";
  for (const auto& token : tokens) {
    std::cout << token.text;
  }
  std::cout << "\"\n";
}

bool is_correct_command(const std::string& line, std::string& out_sentence) {
  std::string t = trim(line);
  if (t.empty()) return false;
  std::string lower = to_lower(t);
  if (lower.size() > 8 && lower.substr(0, 8) == "correct ") {
    out_sentence = trim(t.substr(8));
    return !out_sentence.empty();
  }
  if (lower.size() >= 7 && lower.substr(0, 7) == "rework ") {
    return false;
  }
  if (t.find(' ') != std::string::npos) {
    size_t word_count = 0;
    std::string word;
    std::istringstream iss(t);
    while (iss >> word) {
      word_count++;
    }
    if (word_count > 1) {
      out_sentence = t;
      return true;
    }
  }
  return false;
}

bool is_rework_command(const std::string& line, std::string& out_sentence) {
  std::string t = trim(line);
  if (t.empty()) return false;
  std::string lower = to_lower(t);
  if (lower.size() > 7 && lower.substr(0, 7) == "rework ") {
    out_sentence = trim(t.substr(7));
    return !out_sentence.empty();
  }
  return false;
}

struct ReworkWord {
  std::string original;
  std::string typed;
  bool correct;
};

void run_rework_mode(const std::string& sentence,
                     spell::SpellEngine* engine,
                     spell::DefinitionStore* defs) {
  std::vector<std::string> original_words = split_into_words(sentence);
  
  if (original_words.empty()) {
    std::cout << "No text to rework.\n";
    return;
  }
  
  std::vector<ReworkWord> rework_words;
  for (const auto& w : original_words) {
    ReworkWord rw;
    rw.original = w;
    rw.typed = "";
    rw.correct = false;
    rework_words.push_back(rw);
  }
  
  size_t current_word_idx = 0;
  
  std::cout << "\nRework: \"" << sentence << "\"\n";
  std::cout << "─────────────────────────────────\n";
  std::cout << "Type the sentence (Space to confirm word, Enter when complete, Ctrl+C to cancel):\n\n";
  
  while (current_word_idx < rework_words.size()) {
    for (size_t i = 0; i < rework_words.size(); ++i) {
      if (i > 0) std::cout << " ";
      if (i == current_word_idx) {
        std::cout << "[";
      }
      if (rework_words[i].correct) {
        std::cout << rework_words[i].original;
      } else if (!rework_words[i].typed.empty()) {
        term_bold_red(std::cout, rework_words[i].typed);
      } else {
        std::cout << rework_words[i].original;
      }
      if (i == current_word_idx) {
        std::cout << "]";
      }
    }
    std::cout << "\n";
    
    if (current_word_idx < rework_words.size()) {
      auto& current = rework_words[current_word_idx];
      std::string lower = to_lower_word(current.typed);
      if (!lower.empty() && !engine->is_correct(lower)) {
        auto suggestions = engine->suggest(lower);
        if (!suggestions.empty()) {
          std::cout << "  Suggestions: ";
          size_t show_count = std::min(suggestions.size(), size_t(5));
          for (size_t j = 0; j < show_count; ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << (j + 1) << ":";
            if (!defs->lookup(suggestions[j].word).empty())
              term_bold_yellow(std::cout, suggestions[j].word);
            else
              term_bold(std::cout, suggestions[j].word);
          }
          std::cout << "\n";
          Definition d = defs->lookup(suggestions[0].word);
          if (!d.empty()) {
            std::cout << "  Definition: ";
            term_print_definition(std::cout, d);
          }
        }
      }
    }
    
    std::cout << "\n> " << std::flush;
    
    std::string input;
    if (!std::getline(std::cin, input)) {
      std::cout << "\nCancelled.\n";
      return;
    }
    
    if (input.empty()) {
      continue;
    }
    
    if (input == "\x03" || input == "\x1b") {
      std::cout << "\nCancelled.\n";
      return;
    }
    
    if (input == "\t") {
      if (current_word_idx < rework_words.size()) {
        auto& current = rework_words[current_word_idx];
        std::string lower = to_lower_word(current.typed);
        auto suggestions = engine->suggest(lower);
        if (!suggestions.empty()) {
          current.typed = suggestions[0].word;
        }
      }
      continue;
    }
    
    if (input == "\n" || input == "\r") {
      if (current_word_idx < rework_words.size()) {
        auto& current = rework_words[current_word_idx];
        std::string lower = to_lower_word(current.typed);
        if (current.typed.empty()) {
          if (engine->is_correct(to_lower_word(current.original))) {
            current.correct = true;
            current_word_idx++;
          }
        } else if (engine->is_correct(lower)) {
          current.original = current.typed;
          current.correct = true;
          current_word_idx++;
        } else {
          auto suggestions = engine->suggest(lower);
          if (!suggestions.empty()) {
            current.original = suggestions[0].word;
            current.typed = suggestions[0].word;
            current.correct = true;
            current_word_idx++;
          } else {
            current.correct = true;
            current_word_idx++;
          }
        }
      }
      if (current_word_idx >= rework_words.size()) {
        break;
      }
      continue;
    }
    
    if (input == " " || input == "  ") {
      if (current_word_idx < rework_words.size()) {
        auto& current = rework_words[current_word_idx];
        std::string lower = to_lower_word(current.typed);
        if (engine->is_correct(lower)) {
          current.original = current.typed;
          current.correct = true;
          current_word_idx++;
        } else {
          auto suggestions = engine->suggest(lower);
          if (!suggestions.empty()) {
            current.original = suggestions[0].word;
            current.typed = suggestions[0].word;
            current.correct = true;
            current_word_idx++;
          } else {
            current.correct = true;
            current_word_idx++;
          }
        }
      }
      continue;
    }
    
    if (current_word_idx < rework_words.size()) {
      rework_words[current_word_idx].typed += input;
    }
  }
  
  std::cout << "\nCorrected: \"";
  for (size_t i = 0; i < rework_words.size(); ++i) {
    if (i > 0) std::cout << " ";
    std::cout << (rework_words[i].correct ? rework_words[i].original : rework_words[i].typed);
  }
  std::cout << "\"\n";
}

}  // namespace

int run_repl(const ReplConfig& config) {
#ifndef SPELL_DEFAULT_DICT_DIR
#define SPELL_DEFAULT_DICT_DIR "data/dict"
#endif
  std::string raw_dict = config.dict_dir.empty() ? SPELL_DEFAULT_DICT_DIR : config.dict_dir;
  auto parsed = parse_dict_path(raw_dict);
  std::string dict_dir = parsed.first.empty() ? SPELL_DEFAULT_DICT_DIR : parsed.first;
  std::string dict_base = parsed.second;
  std::string user_dict = config.user_dict_path;
  auto engine = HunspellEngine::create(dict_dir, dict_base, user_dict);

  if (!engine->is_loaded()) {
#ifndef SPELL_HAS_HUNSPELL
    std::cout << "spell: Spell checking is disabled — this build was compiled without Hunspell.\n";
    std::cout << "  Install libhunspell-dev (or equivalent), then rebuild spell to enable checking.\n\n";
#else
    std::cout << "spell: Could not load dictionary from " << dict_dir << "\n";
    std::cout << "  Try: load /path/to/dict  (in REPL)\n";
    std::cout << "  Or:  spell --dict-dir /path/to/dict\n";
    std::cout << "  Config: ~/.config/spell/config or ~/.spellrc with dict_dir=/path/to/dict\n\n";
#endif
    std::cout << "  Starting REPL without spell check (type 'help' for commands).\n\n";
    engine.reset();
  } else {
    std::cout << "Dictionary: " << dict_dir << " (en_US)\n";
  }

  std::string defs_path = config.defs_path;
#ifdef SPELL_DEFAULT_DEFS
  if (defs_path.empty()) defs_path = SPELL_DEFAULT_DEFS;
#endif
  std::unique_ptr<DefinitionStore> defs;
  if (!defs_path.empty()) {
    defs = FileDefinitionStore::load(defs_path);
    if (defs)
      std::cout << "Definitions: " << defs_path << "\n";
  }
  if (!defs)
    defs = std::make_unique<StubDefinitionStore>();

  std::cout << "spell - Interactive spell checker (type 'help' or '?' for help)\n\n";

#ifdef SPELL_HAS_READLINE
  rl_attempted_completion_function = repl_completion;
  rl_basic_word_break_characters = " \t\n";
#endif

  while (true) {
    char* line_cstr = nullptr;
#ifdef SPELL_HAS_READLINE
    bool use_readline = isatty(STDIN_FILENO);
    if (use_readline) {
      line_cstr = readline(prompt);
    } else {
      std::cout << prompt << std::flush;
      std::string line_str;
      if (!std::getline(std::cin, line_str)) break;
      line_cstr = strdup(line_str.c_str());
    }
#else
    bool use_readline = false;
    std::cout << prompt << std::flush;
    std::string line_str;
    if (!std::getline(std::cin, line_str)) break;
    line_cstr = strdup(line_str.c_str());
#endif

    if (!line_cstr) break;
    std::string line(line_cstr);
    free(line_cstr);

#ifdef SPELL_HAS_READLINE
    if (use_readline && !line.empty()) add_history(line.c_str());
#endif

    if (is_quit_command(line)) break;
    if (is_help_command(line)) {
      std::cout << repl_help;
      continue;
    }

    std::string load_path;
    if (is_load_command(line, load_path)) {
      auto parsed = parse_dict_path(load_path);
      if (parsed.first.empty()) {
        std::cout << "Invalid path.\n";
        continue;
      }
      engine = HunspellEngine::create(parsed.first, parsed.second, user_dict);
      if (engine->is_loaded()) {
        dict_dir = parsed.first;
        dict_base = parsed.second;
        std::cout << "Loaded dictionary: " << dict_dir << " (" << dict_base << ")\n";
      } else {
        std::cout << "Could not load dictionary from " << load_path << "\n";
        engine.reset();
      }
      continue;
    }

    if (is_dict_command(line)) {
      if (engine && engine->is_loaded()) {
        std::cout << "Dictionary: " << dict_dir << " (" << dict_base << ")\n";
      } else {
        std::cout << "No dictionary loaded. Use 'load PATH' to load one.\n";
      }
      continue;
    }

    std::string define_word;
    if (is_define_command(line, define_word)) {
      Definition d = defs->lookup(define_word);
      if (d.empty()) {
        std::cout << "No definition for \"" << define_word << "\". (Use --defs or defs_path for a glossary.)\n";
      } else {
        term_print_definition(std::cout, d);
      }
      continue;
    }

    std::string sentence;
    if (is_correct_command(line, sentence)) {
      if (!engine || !engine->is_loaded()) {
#ifndef SPELL_HAS_HUNSPELL
        std::cout << "(spell check unavailable — build has no Hunspell; install libhunspell-dev and rebuild)\n";
#else
        std::cout << "(no dictionary — use 'load PATH', --dict-dir, or set dict_dir in ~/.config/spell/config)\n";
#endif
        continue;
      }
      correct_sentence(sentence, engine.get(), defs.get());
      continue;
    }

    if (is_rework_command(line, sentence)) {
      if (!engine || !engine->is_loaded()) {
#ifndef SPELL_HAS_HUNSPELL
        std::cout << "(spell check unavailable — build has no Hunspell; install libhunspell-dev and rebuild)\n";
#else
        std::cout << "(no dictionary — use 'load PATH', --dict-dir, or set dict_dir in ~/.config/spell/config)\n";
#endif
        continue;
      }
      run_rework_mode(sentence, engine.get(), defs.get());
      continue;
    }

    std::string word = extract_word(line);
    if (word.empty()) continue;

    if (!engine) {
#ifndef SPELL_HAS_HUNSPELL
      std::cout << "(spell check unavailable — build has no Hunspell; install libhunspell-dev and rebuild)\n";
#else
      std::cout << "(no dictionary — use 'load PATH', --dict-dir, or set dict_dir in ~/.config/spell/config)\n";
#endif
      continue;
    }

    if (engine->is_correct(word)) {
      std::cout << random_correct_affirmation() << "\n";
      Definition d = defs->lookup(word);
      if (!d.empty())
        term_print_definition_inline(std::cout, word, d);
    } else {
      auto suggestions = engine->suggest(word);
      if (suggestions.size() > config.max_suggestions) {
        suggestions.resize(config.max_suggestions);
      }
      if (suggestions.empty()) {
        std::cout << word << ": no suggestions\n";
      } else {
#ifdef SPELL_HAS_READLINE
        last_suggestion_words.clear();
        for (size_t i = 0; i < suggestions.size(); ++i)
          last_suggestion_words.push_back(suggestions[i].word);
#endif
        std::cout << "Did you mean: ";
        for (size_t i = 0; i < suggestions.size(); ++i) {
          if (i > 0) std::cout << ", ";
          if (!defs->lookup(suggestions[i].word).empty())
            term_bold_yellow(std::cout, suggestions[i].word);
          else
            term_bold(std::cout, suggestions[i].word);
        }
        std::cout << "?\n";
        for (size_t i = 0; i < suggestions.size(); ++i) {
          Definition d = defs->lookup(suggestions[i].word);
          if (!d.empty())
            term_print_definition(std::cout, d);
        }
      }
    }
  }

  return 0;
}

}  // namespace spell
