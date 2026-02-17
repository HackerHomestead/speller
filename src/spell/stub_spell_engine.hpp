#pragma once

#include "spell/spell_engine.hpp"
#include <map>
#include <set>

namespace spell {

/// Stub implementation for testing. All words in dictionary are correct;
/// suggestions are configurable.
class StubSpellEngine : public SpellEngine {
 public:
  void add_correct_word(const std::string& word);
  void add_suggestion(const std::string& misspelling, const std::string& suggestion, float score = 1.0f);

  bool is_correct(const std::string& word) override;
  std::vector<Suggestion> suggest(const std::string& word) override;

 private:
  std::set<std::string> correct_words_;
  std::map<std::string, std::vector<Suggestion>> suggestions_;
};

}  // namespace spell
