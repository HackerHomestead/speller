#include <catch2/catch_test_macros.hpp>
#include "spell/suggestion_orchestrator.hpp"
#include "spell/stub_spell_engine.hpp"

using namespace spell;

TEST_CASE("SuggestionOrchestrator merges suggestions from multiple engines",
          "[suggestion_orchestrator]") {
  auto e1 = std::make_unique<StubSpellEngine>();
  e1->add_suggestion("helo", "hello", 1.0f);
  e1->add_suggestion("helo", "help", 0.5f);

  auto e2 = std::make_unique<StubSpellEngine>();
  e2->add_suggestion("helo", "hello", 0.8f);
  e2->add_suggestion("helo", "held", 0.3f);

  std::vector<std::unique_ptr<SpellEngine>> engines;
  engines.push_back(std::move(e1));
  engines.push_back(std::move(e2));

  SuggestionOrchestrator orch(std::move(engines));
  auto suggestions = orch.suggest("helo", 5);

  REQUIRE(suggestions.size() >= 3);
  // hello should have highest score (1.0 + 0.8)
  REQUIRE(suggestions[0].word == "hello");
  REQUIRE(suggestions[0].score > 1.5f);
}

TEST_CASE("SuggestionOrchestrator limits to max_count", "[suggestion_orchestrator]") {
  auto e = std::make_unique<StubSpellEngine>();
  e->add_suggestion("x", "xa", 1.0f);
  e->add_suggestion("x", "xb", 0.9f);
  e->add_suggestion("x", "xc", 0.8f);
  e->add_suggestion("x", "xd", 0.7f);
  e->add_suggestion("x", "xe", 0.6f);

  std::vector<std::unique_ptr<SpellEngine>> engines;
  engines.push_back(std::move(e));

  SuggestionOrchestrator orch(std::move(engines));
  auto suggestions = orch.suggest("x", 3);

  REQUIRE(suggestions.size() == 3);
}

TEST_CASE("SuggestionOrchestrator with empty engines returns empty", "[suggestion_orchestrator]") {
  std::vector<std::unique_ptr<SpellEngine>> engines;
  SuggestionOrchestrator orch(std::move(engines));
  auto suggestions = orch.suggest("helo", 5);
  REQUIRE(suggestions.empty());
}

TEST_CASE("SuggestionOrchestrator with engine returning nothing", "[suggestion_orchestrator]") {
  auto e = std::make_unique<StubSpellEngine>();
  std::vector<std::unique_ptr<SpellEngine>> engines;
  engines.push_back(std::move(e));
  SuggestionOrchestrator orch(std::move(engines));
  auto suggestions = orch.suggest("unknown", 5);
  REQUIRE(suggestions.empty());
}

TEST_CASE("SuggestionOrchestrator deduplicates and sums scores", "[suggestion_orchestrator]") {
  auto e = std::make_unique<StubSpellEngine>();
  e->add_suggestion("x", "same", 1.0f);
  e->add_suggestion("x", "same", 0.5f);  // same word again from same engine
  std::vector<std::unique_ptr<SpellEngine>> engines;
  engines.push_back(std::move(e));
  SuggestionOrchestrator orch(std::move(engines));
  auto suggestions = orch.suggest("x", 5);
  REQUIRE(suggestions.size() == 1);
  REQUIRE(suggestions[0].word == "same");
  REQUIRE(suggestions[0].score == 1.5f);
}
