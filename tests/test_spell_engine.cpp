#include <catch2/catch_test_macros.hpp>
#include "spell/spell_engine.hpp"
#include "spell/stub_spell_engine.hpp"

using namespace spell;

TEST_CASE("StubSpellEngine reports correct words", "[spell_engine]") {
  StubSpellEngine engine;
  engine.add_correct_word("hello");
  engine.add_correct_word("world");

  REQUIRE(engine.is_correct("hello"));
  REQUIRE(engine.is_correct("world"));
  REQUIRE_FALSE(engine.is_correct("helo"));
  REQUIRE_FALSE(engine.is_correct(""));
}

TEST_CASE("StubSpellEngine returns configured suggestions", "[spell_engine]") {
  StubSpellEngine engine;
  engine.add_suggestion("helo", "hello", 1.0f);
  engine.add_suggestion("helo", "help", 0.5f);

  auto suggestions = engine.suggest("helo");
  REQUIRE(suggestions.size() == 2);
  REQUIRE(suggestions[0].word == "hello");
  REQUIRE(suggestions[0].score == 1.0f);
  REQUIRE(suggestions[1].word == "help");
  REQUIRE(suggestions[1].score == 0.5f);
}

TEST_CASE("StubSpellEngine returns empty suggestions for unknown word", "[spell_engine]") {
  StubSpellEngine engine;
  auto suggestions = engine.suggest("xyznonexistent");
  REQUIRE(suggestions.empty());
}
