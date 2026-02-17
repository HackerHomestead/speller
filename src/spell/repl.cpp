#include "spell/repl.hpp"
#include "spell/file_definition_store.hpp"
#include "spell/hunspell_engine.hpp"
#include "spell/stub_definition_store.hpp"
#include "util/dict_path.hpp"
#include "util/term_format.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <memory>

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
    "  Type a word and press Enter to check spelling.\n"
    "  Commands (start with : or use full name):\n\n"
    "    help, ?        Show this help\n"
    "    load PATH      Load dictionary (PATH = dir or path to .aff file)\n"
    "    :load PATH     Same as load\n"
    "    dict           Show current dictionary path\n"
    "    define WORD    Show definition for WORD (from glossary)\n"
    "    def WORD       Same as define\n"
    "    quit, exit     Exit the REPL\n"
    "    :q             Shortcut to quit\n\n"
    "  Examples:\n\n"
    "    spell> hello\n"
    "    OK\n\n"
    "    spell> helo\n"
    "    Did you mean: hello?\n\n"
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
const char* repl_commands[] = {"help", "?", "load", ":load", "dict", "define", "def", "quit", "exit", ":q", nullptr};

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

char** repl_completion(const char* text, int start, int end) {
  (void)end;
  if (start == 0) {
    return rl_completion_matches(text, repl_command_generator);
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
  return t;
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
      std::cout << "OK\n";
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
        std::cout << "Did you mean: ";
        for (size_t i = 0; i < suggestions.size(); ++i) {
          if (i > 0) std::cout << ", ";
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
