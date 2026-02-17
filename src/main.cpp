#include "spell/hunspell_engine.hpp"
#include "spell/repl.hpp"
#include "spell/suggestion_orchestrator.hpp"
#include "util/config.hpp"
#include <iostream>
#include <cstdlib>

namespace {

const char* help_text =
    "spell - Offline spell checker and definition tool\n"
    "\n"
    "Usage: spell [options]     Run with no args for interactive REPL\n"
    "\n"
    "Options:\n"
    "  (no args)        Interactive REPL (default)\n"
    "  --interactive    Same as default\n"
    "  --stream         Stream mode: read from stdin/file, write to stdout\n"
    "  --file PATH      Read from file (implies --stream)\n"
    "  --check WORD     Check a single word and show suggestions\n"
    "  --dict-dir PATH  Dictionary directory (.aff, .dic files)\n"
    "  --user-dict PATH User dictionary (one word per line)\n"
    "  --defs PATH      Definition database path\n"
    "  --fast           Auto-apply top suggestion in stream mode\n"
    "  --careful        Prompt when confidence is low (default)\n"
    "  -h, --help       Show this help\n"
    "  -V, --version    Show version and build info\n";

int run_spell(const spell::Config& config) {
  // --check: quick single-word check (requires --dict-dir)
  if (!config.check_word.empty()) {
    std::string dict_dir = config.dict_dir.empty() ? "/usr/share/hunspell" : config.dict_dir;
    auto engine = spell::HunspellEngine::create(dict_dir, "en_US", config.user_dict_path);
    if (!engine->is_loaded()) {
      std::cerr << "spell: Could not load dictionary from " << dict_dir << "\n";
      return 1;
    }
    if (engine->is_correct(config.check_word)) {
      std::cout << config.check_word << ": OK\n";
      return 0;
    }
    std::vector<std::unique_ptr<spell::SpellEngine>> engines;
    engines.push_back(std::move(engine));
    spell::SuggestionOrchestrator orchestrator(std::move(engines));
    auto suggestions = orchestrator.suggest(config.check_word, config.max_suggestions);
    std::cout << config.check_word << ": ";
    if (suggestions.empty()) {
      std::cout << "no suggestions\n";
    } else {
      std::cout << "Did you mean: ";
      for (size_t i = 0; i < suggestions.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << suggestions[i].word;
      }
      std::cout << "?\n";
    }
    return 0;
  }

  spell::ReplConfig repl_config;
  repl_config.dict_dir = config.dict_dir;
  repl_config.user_dict_path = config.user_dict_path;
  repl_config.max_suggestions = config.max_suggestions;
  return spell::run_repl(repl_config);
}

}  // namespace

int main(int argc, char* argv[]) {
  auto config = spell::Config::from_args(argc, argv);

  if (config.help_requested) {
    std::cout << help_text;
    return 0;
  }

  if (config.version_requested) {
    std::cout << "spell " << SPELL_VERSION << "\n";
#ifdef SPELL_HAS_HUNSPELL
    std::cout << "  Hunspell: enabled\n";
#else
    std::cout << "  Hunspell: disabled (install libhunspell-dev to enable)\n";
#endif
    return 0;
  }

  return run_spell(config);
}
