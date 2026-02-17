#include "spell/stub_spell_engine.hpp"

namespace spell {

void StubSpellEngine::add_correct_word(const std::string& word) {
  correct_words_.insert(word);
}

void StubSpellEngine::add_suggestion(const std::string& misspelling,
                                     const std::string& suggestion,
                                     float score) {
  suggestions_[misspelling].push_back({suggestion, score});
}

bool StubSpellEngine::is_correct(const std::string& word) {
  return correct_words_.count(word) > 0;
}

std::vector<Suggestion> StubSpellEngine::suggest(const std::string& word) {
  auto it = suggestions_.find(word);
  if (it == suggestions_.end()) return {};
  return it->second;
}

}  // namespace spell
