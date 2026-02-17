#include <catch2/catch_test_macros.hpp>

#ifdef SPELL_HAS_HUNSPELL
#include "spell/hunspell_engine.hpp"
#include <cstdlib>
#include <unistd.h>

using namespace spell;

namespace {
std::string find_dict_dir() {
  const char* dirs[] = {
    "/usr/share/hunspell",
    "/usr/share/myspell/dicts",
    nullptr
  };
  for (int i = 0; dirs[i]; ++i) {
    if (access((std::string(dirs[i]) + "/en_US.aff").c_str(), R_OK) == 0) {
      return dirs[i];
    }
  }
  return "";
}
}

TEST_CASE("HunspellEngine create returns engine", "[hunspell_engine]") {
  auto engine = HunspellEngine::create("/nonexistent", "en_US");
  REQUIRE(engine != nullptr);
  REQUIRE_FALSE(engine->is_loaded());
}

TEST_CASE("HunspellEngine is_correct with valid dict", "[hunspell_engine]") {
  std::string dict_dir = find_dict_dir();
  if (dict_dir.empty()) {
    SKIP("No Hunspell dictionary found (install hunspell-en-us)");
  }

  auto engine = HunspellEngine::create(dict_dir, "en_US");
  REQUIRE(engine->is_loaded());

  REQUIRE(engine->is_correct("hello"));
  REQUIRE(engine->is_correct("world"));
  REQUIRE_FALSE(engine->is_correct("helo"));
}

TEST_CASE("HunspellEngine suggest returns corrections", "[hunspell_engine]") {
  std::string dict_dir = find_dict_dir();
  if (dict_dir.empty()) {
    SKIP("No Hunspell dictionary found (install hunspell-en-us)");
  }

  auto engine = HunspellEngine::create(dict_dir, "en_US");
  REQUIRE(engine->is_loaded());

  auto suggestions = engine->suggest("helo");
  REQUIRE_FALSE(suggestions.empty());
  bool has_hello = false;
  for (const auto& s : suggestions) {
    if (s.word == "hello") has_hello = true;
  }
  REQUIRE(has_hello);
}

#else  // !SPELL_HAS_HUNSPELL

TEST_CASE("HunspellEngine skipped - Hunspell not available", "[hunspell_engine]") {
  // Build without libhunspell-dev; Hunspell tests are skipped
  REQUIRE(true);
}

#endif  // SPELL_HAS_HUNSPELL
