#pragma once

#include "spell/spell_engine.hpp"
#include <memory>
#include <vector>

namespace spell {

class SuggestionOrchestrator {
 public:
  explicit SuggestionOrchestrator(std::vector<std::unique_ptr<SpellEngine>> engines);
  ~SuggestionOrchestrator();

  SuggestionOrchestrator(SuggestionOrchestrator&&) = default;
  SuggestionOrchestrator& operator=(SuggestionOrchestrator&&) = default;
  SuggestionOrchestrator(const SuggestionOrchestrator&) = delete;
  SuggestionOrchestrator& operator=(const SuggestionOrchestrator&) = delete;

  std::vector<Suggestion> suggest(const std::string& word, size_t max_count = 5);

 private:
  std::vector<std::unique_ptr<SpellEngine>> engines_;
};

}  // namespace spell
