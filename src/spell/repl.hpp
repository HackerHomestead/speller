#pragma once

#include "spell/spell_engine.hpp"
#include "spell/suggestion_orchestrator.hpp"
#include <memory>
#include <string>

namespace spell {

struct ReplConfig {
  std::string dict_dir;
  std::string user_dict_path;
  std::string defs_path;  // Glossary file for definitions (optional)
  size_t max_suggestions = 5;
};

/// Run the interactive REPL. Returns exit code.
int run_repl(const ReplConfig& config);

}  // namespace spell
