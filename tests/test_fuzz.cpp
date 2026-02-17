#include <catch2/catch_test_macros.hpp>
#include "fuzz_utils.hpp"
#include "spell/spell_engine.hpp"
#include "spell/stub_spell_engine.hpp"
#include "spell/suggestion_orchestrator.hpp"
#include "spell/hunspell_engine.hpp"
#include <string>
#include <vector>
#include <set>

using namespace spell;

namespace {

const std::vector<std::string> CORRECT_WORDS = {
  "hello", "world", "test", "correct", "dictionary", "spelling", "check",
  "word", "the", "and", "for", "with", "from", "that", "this", "have",
  "been", "were", "their", "there", "they", "would", "could", "should",
  "about", "which", "when", "where", "what", "who", "how", "some", "more",
  "other", "over", "after", "before", "first", "second", "best", "most",
  "great", "small", "good", "bad", "new", "old", "long", "short", "right",
  "wrong", "true", "false", "same", "different", "another", "each", "every",
  "all", "both", "few", "many", "very", "too", "also", "only", "just",
  "even", "still", "always", "never", "often", "really", "actually",
  "clearly", "obviously", "probably", "certainly", "definitely",
  "completely", "entirely", "totally", "fully", "mainly", "generally",
  "especially", "particularly", "specifically", "therefore", "however",
  "nevertheless", "meanwhile", "furthermore", "moreover", "instead",
  "rather", "quite", "fairly", "somewhat", "slightly", "nearly", "almost"
};

}  // namespace

TEST_CASE("dyslexic misspellings - StubSpellEngine does not crash", "[fuzz][dyslexic]") {
  auto engine = std::make_unique<StubSpellEngine>();
  for (const auto& w : CORRECT_WORDS) engine->add_correct_word(w);
  for (const auto& w : CORRECT_WORDS) engine->add_suggestion("helo", "hello", 1.0f);

  auto misspellings = fuzz::generate_dyslexic_misspellings(CORRECT_WORDS, 100, 42);
  REQUIRE(misspellings.size() == 100);

  for (const auto& word : misspellings) {
    (void)engine->is_correct(word);
    auto suggestions = engine->suggest(word);
    (void)suggestions;
  }
}

TEST_CASE("fuzz words - StubSpellEngine does not crash", "[fuzz][fuzzing]") {
  auto engine = std::make_unique<StubSpellEngine>();
  for (const auto& w : CORRECT_WORDS) engine->add_correct_word(w);

  auto fuzz_words = fuzz::generate_fuzz_words(100, 12345);
  REQUIRE(fuzz_words.size() == 100);

  for (const auto& word : fuzz_words) {
    (void)engine->is_correct(word);
    auto suggestions = engine->suggest(word);
    (void)suggestions;
  }
}

TEST_CASE("dyslexic misspellings - SuggestionOrchestrator does not crash", "[fuzz][dyslexic]") {
  auto engine = std::make_unique<StubSpellEngine>();
  for (const auto& w : CORRECT_WORDS) engine->add_correct_word(w);
  for (const auto& m : {"helo", "wrold", "teh", "adn", "taht"}) {
    engine->add_suggestion(m, "hello", 0.9f);
    engine->add_suggestion(m, "world", 0.9f);
    engine->add_suggestion(m, "the", 0.9f);
    engine->add_suggestion(m, "and", 0.9f);
    engine->add_suggestion(m, "that", 0.9f);
  }

  std::vector<std::unique_ptr<SpellEngine>> engines;
  engines.push_back(std::move(engine));
  SuggestionOrchestrator orch(std::move(engines));

  auto misspellings = fuzz::generate_dyslexic_misspellings(CORRECT_WORDS, 100, 999);
  for (const auto& word : misspellings) {
    auto suggestions = orch.suggest(word, 5);
    (void)suggestions;
  }
}

TEST_CASE("fuzz words - SuggestionOrchestrator does not crash", "[fuzz][fuzzing]") {
  auto engine = std::make_unique<StubSpellEngine>();
  std::vector<std::unique_ptr<SpellEngine>> engines;
  engines.push_back(std::move(engine));
  SuggestionOrchestrator orch(std::move(engines));

  auto fuzz_words = fuzz::generate_fuzz_words(100, 67890);
  for (const auto& word : fuzz_words) {
    auto suggestions = orch.suggest(word, 5);
    (void)suggestions;
  }
}

#ifdef SPELL_HAS_HUNSPELL
TEST_CASE("dyslexic misspellings - HunspellEngine with bundled dict", "[fuzz][dyslexic][hunspell]") {
  std::string dict_dir = SPELL_DICT_DIR;
  auto engine = HunspellEngine::create(dict_dir, "en_US");
  if (!engine->is_loaded()) {
    SKIP("Hunspell dictionary not available");
  }

  auto misspellings = fuzz::generate_dyslexic_misspellings(CORRECT_WORDS, 100, 111);
  size_t with_suggestions = 0;
  size_t suggestion_contains_correct = 0;
  std::set<std::string> correct_set(CORRECT_WORDS.begin(), CORRECT_WORDS.end());

  for (const auto& misspelled : misspellings) {
    (void)engine->is_correct(misspelled);
    auto suggestions = engine->suggest(misspelled);
    if (!suggestions.empty()) {
      ++with_suggestions;
      for (const auto& s : suggestions) {
        if (correct_set.count(s.word)) {
          ++suggestion_contains_correct;
          break;
        }
      }
    }
  }

  REQUIRE(misspellings.size() == 100);
  REQUIRE(with_suggestions > 0);
  REQUIRE(suggestion_contains_correct > 0);
}

TEST_CASE("fuzz words - HunspellEngine does not crash", "[fuzz][fuzzing][hunspell]") {
  std::string dict_dir = SPELL_DICT_DIR;
  auto engine = HunspellEngine::create(dict_dir, "en_US");
  if (!engine->is_loaded()) {
    SKIP("Hunspell dictionary not available");
  }

  auto fuzz_words = fuzz::generate_fuzz_words(100, 22222);
  for (const auto& word : fuzz_words) {
    (void)engine->is_correct(word);
    auto suggestions = engine->suggest(word);
    (void)suggestions;
  }
  REQUIRE(fuzz_words.size() == 100);
}
#else
TEST_CASE("fuzz Hunspell tests skipped - no Hunspell", "[fuzz]") {
  REQUIRE(true);
}
#endif
