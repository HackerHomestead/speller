#include "spell/suggestion_orchestrator.hpp"
#include <algorithm>
#include <map>

namespace spell {

SuggestionOrchestrator::SuggestionOrchestrator(
    std::vector<std::unique_ptr<SpellEngine>> engines)
    : engines_(std::move(engines)) {}

SuggestionOrchestrator::~SuggestionOrchestrator() = default;

std::vector<Suggestion> SuggestionOrchestrator::suggest(const std::string& word,
                                                       size_t max_count) {
  std::map<std::string, float> merged;

  for (auto& engine : engines_) {
    for (const auto& s : engine->suggest(word)) {
      merged[s.word] += s.score;
    }
  }

  std::vector<Suggestion> result;
  for (const auto& [w, score] : merged) {
    result.push_back({w, score});
  }
  std::sort(result.begin(), result.end(),
            [](const Suggestion& a, const Suggestion& b) { return a.score > b.score; });

  if (result.size() > max_count) {
    result.resize(max_count);
  }
  return result;
}

}  // namespace spell
