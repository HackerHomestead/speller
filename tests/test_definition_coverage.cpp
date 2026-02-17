#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "spell/file_definition_store.hpp"

// Returns list of words in a Hunspell .dic file (first line = count, then one word per line;
// word may have /flags, we take the stem only).
static std::vector<std::string> read_dic_words(const std::string& path) {
  std::vector<std::string> words;
  std::ifstream f(path);
  if (!f) return words;
  std::string line;
  if (!std::getline(f, line)) return words;  // skip count line
  while (std::getline(f, line)) {
    size_t slash = line.find('/');
    std::string word = slash != std::string::npos ? line.substr(0, slash) : line;
    // trim trailing space (Hunspell can have space before /)
    while (!word.empty() && (word.back() == ' ' || word.back() == '\t')) word.pop_back();
    if (!word.empty()) words.push_back(word);
  }
  return words;
}

// Definition coverage: fraction of words in the bundled dictionary that have
// at least one entry in the glossary. We aim for high coverage so users see
// definitions for common words.
TEST_CASE("Definition coverage of bundled English dictionary", "[definition_store][coverage]") {
#ifdef SPELL_DICT_DIR
#ifdef SPELL_SOURCE_DIR
  std::string dic_path = std::string(SPELL_DICT_DIR) + "/en_US.dic";
  std::string glossary_path = std::string(SPELL_SOURCE_DIR) + "/data/glossary.txt";

  auto words = read_dic_words(dic_path);
  REQUIRE_FALSE(words.empty());

  auto store = spell::FileDefinitionStore::load(glossary_path);
  REQUIRE(store);

  size_t defined = 0;
  for (const auto& w : words) {
    if (!store->lookup(w).empty()) ++defined;
  }

  double coverage = static_cast<double>(defined) / static_cast<double>(words.size());
  REQUIRE(defined > 0);
  // Require at least 80% of bundled dict words have definitions (glossary should cover English set)
  REQUIRE(coverage >= 0.80);

  // Report for CI and developers
  INFO("Definition coverage: " << defined << "/" << words.size() << " ("
       << static_cast<int>(coverage * 100) << "%)");
#endif
#endif
}
