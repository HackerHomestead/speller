#include <catch2/catch_test_macros.hpp>
#include "spell/definition_store.hpp"
#include "spell/file_definition_store.hpp"
#include "spell/stub_definition_store.hpp"

using namespace spell;

TEST_CASE("Definition empty() when headword is empty", "[definition_store]") {
  Definition def;
  REQUIRE(def.empty());
  def.headword = "test";
  REQUIRE_FALSE(def.empty());
}

TEST_CASE("StubDefinitionStore returns configured definitions", "[definition_store]") {
  StubDefinitionStore store;
  store.add_definition("hello", "n.", "a greeting");

  auto def = store.lookup("hello");
  REQUIRE_FALSE(def.empty());
  REQUIRE(def.headword == "hello");
  REQUIRE(def.part_of_speech == "n.");
  REQUIRE(def.short_gloss == "a greeting");
}

TEST_CASE("StubDefinitionStore returns empty for unknown word", "[definition_store]") {
  StubDefinitionStore store;
  auto def = store.lookup("xyznonexistent");
  REQUIRE(def.empty());
}

TEST_CASE("StubDefinitionStore overwrites with add_definition", "[definition_store]") {
  StubDefinitionStore store;
  store.add_definition("test", "n.", "first");
  store.add_definition("test", "v.", "second");
  auto def = store.lookup("test");
  REQUIRE(def.part_of_speech == "v.");
  REQUIRE(def.short_gloss == "second");
}

TEST_CASE("Definition empty when headword empty", "[definition_store]") {
  Definition def;
  def.part_of_speech = "n.";
  def.short_gloss = "something";
  REQUIRE(def.empty());
}

TEST_CASE("FileDefinitionStore load and lookup", "[definition_store]") {
#ifdef SPELL_SOURCE_DIR
  std::string path = std::string(SPELL_SOURCE_DIR) + "/tests/fixtures/glossary.txt";
  auto store = FileDefinitionStore::load(path);
  REQUIRE(store);
  auto def = store->lookup("test");
  REQUIRE_FALSE(def.empty());
  REQUIRE(def.headword == "test");
  REQUIRE(def.part_of_speech == "n.");
  REQUIRE(def.short_gloss == "a trial or experiment");
  REQUIRE(store->lookup("unknown").empty());
  REQUIRE_FALSE(store->lookup("TEST").empty());  // case-insensitive
#endif
}

TEST_CASE("FileDefinitionStore load returns null for missing file", "[definition_store]") {
  auto store = FileDefinitionStore::load("/nonexistent/glossary.txt");
  REQUIRE_FALSE(store);
}
