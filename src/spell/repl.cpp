#include "spell/repl.hpp"
#include "spell/hunspell_engine.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cstring>

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
    "    quit, exit     Exit the REPL\n"
    "    :q             Shortcut to quit\n\n"
    "  Examples:\n\n"
    "    spell> hello\n"
    "    OK\n\n"
    "    spell> helo\n"
    "    Did you mean: hello?\n\n"
    "    spell> help\n"
    "    (shows this help)\n\n"
    "    spell> :q\n"
    "    (exits)\n\n"
    "  Tips:\n"
    "  - Tab completes commands (help, quit, :q)\n"
    "  - Up/Down for command history\n"
    "  - Empty line does nothing\n\n";

const char* prompt = "spell> ";

#ifdef SPELL_HAS_READLINE
const char* repl_commands[] = {"help", "?", "quit", "exit", ":q", nullptr};

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

std::string extract_word(const std::string& line) {
  std::string t = trim(line);
  if (t.empty()) return "";
  if (t[0] == ':') return "";
  std::string lower = to_lower(t);
  if (lower == "help" || lower == "?" || lower == "quit" || lower == "exit" || lower == "q") return "";
  if (lower.size() >= 5 && lower.substr(0, 5) == "check") {
    return trim(t.substr(5));
  }
  return t;
}

}  // namespace

int run_repl(const ReplConfig& config) {
  std::string dict_dir = config.dict_dir.empty() ? "/usr/share/hunspell" : config.dict_dir;
  auto engine = HunspellEngine::create(dict_dir, "en_US", config.user_dict_path);

  if (!engine->is_loaded()) {
    std::cout << "spell: Could not load dictionary.\n";
    std::cout << "  Try: spell --dict-dir /path/to/dict\n";
    std::cout << "  Or:  spell --dict-dir " << dict_dir << "\n\n";
    std::cout << "  Starting REPL without spell check (type 'help' for commands).\n\n";
    engine.reset();
  }

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

    std::string word = extract_word(line);
    if (word.empty()) continue;

    if (!engine) {
      std::cout << "(no dictionary - use --dict-dir to enable spell check)\n";
      continue;
    }

    if (engine->is_correct(word)) {
      std::cout << "OK\n";
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
          std::cout << suggestions[i].word;
        }
        std::cout << "?\n";
      }
    }
  }

  return 0;
}

}  // namespace spell
