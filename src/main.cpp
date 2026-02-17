#include "spell/file_definition_store.hpp"
#include "spell/hunspell_engine.hpp"
#include "spell/repl.hpp"
#include "spell/stub_definition_store.hpp"
#include "spell/suggestion_orchestrator.hpp"
#include "util/config.hpp"
#include "util/dict_path.hpp"
#include "util/term_format.hpp"
#include <iostream>
#include <memory>
#include <cstdlib>

namespace {

const char* help_text =
    "spell - Offline spell checker and definition tool\n"
    "\n"
    "Usage: spell [options]     No args = interactive REPL\n"
    "\n"
    "Dictionary (default: bundled en_US, or set in config):\n"
    "  Config file:  ~/.config/spell/config  or  ~/.spellrc\n"
    "    dict_dir=/path/to/dict   (or path to a .aff file)\n"
    "    user_dict=/path/to/user.dic\n"
    "    defs_path=/path/to/glossary.txt   (for definitions)\n"
    "  CLI:  --dict-dir PATH   (directory or path to .aff file)\n"
    "\n"
    "Options:\n"
    "  (no args)        Interactive REPL\n"
    "  --check WORD     Check one word\n"
    "  --dict-dir PATH  Dictionary dir or .aff file\n"
    "  --user-dict PATH User dictionary\n"
    "  --defs PATH      Glossary file (word<TAB>pos<TAB>definition) for definitions\n"
    "  --file PATH      Read from file\n"
    "  -h, --help       Show this help\n"
    "  -V, --version    Version and build info\n";

int run_spell(const spell::Config& config) {
  // --check: quick single-word check (requires --dict-dir)
  if (!config.check_word.empty()) {
#ifndef SPELL_DEFAULT_DICT_DIR
#define SPELL_DEFAULT_DICT_DIR "data/dict"
#endif
    std::string raw_dict = config.dict_dir.empty() ? SPELL_DEFAULT_DICT_DIR : config.dict_dir;
    auto parsed = spell::parse_dict_path(raw_dict);
    if (parsed.first.empty()) parsed.first = SPELL_DEFAULT_DICT_DIR;
    auto engine = spell::HunspellEngine::create(parsed.first, parsed.second, config.user_dict_path);
    if (!engine->is_loaded()) {
      std::cerr << "spell: Could not load dictionary from " << parsed.first << "\n";
      return 1;
    }
    std::string defs_path = config.defs_path;
#ifdef SPELL_DEFAULT_DEFS
    if (defs_path.empty()) defs_path = SPELL_DEFAULT_DEFS;
#endif
    std::unique_ptr<spell::DefinitionStore> defs;
    if (!defs_path.empty()) {
      defs = spell::FileDefinitionStore::load(defs_path);
    }
    if (!defs)
      defs = std::make_unique<spell::StubDefinitionStore>();

    if (engine->is_correct(config.check_word)) {
      std::cout << config.check_word << ": OK\n";
      spell::Definition d = defs->lookup(config.check_word);
      if (!d.empty())
        spell::term_print_definition_inline(std::cout, config.check_word, d);
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
        spell::term_bold(std::cout, suggestions[i].word);
      }
      std::cout << "?\n";
      for (size_t i = 0; i < suggestions.size(); ++i) {
        spell::Definition d = defs->lookup(suggestions[i].word);
        if (!d.empty())
          spell::term_print_definition(std::cout, d);
      }
    }
    return 0;
  }

  spell::ReplConfig repl_config;
  repl_config.dict_dir = config.dict_dir;
  repl_config.user_dict_path = config.user_dict_path;
  repl_config.defs_path = config.defs_path;
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
