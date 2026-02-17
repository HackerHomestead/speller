#include <catch2/catch_test_macros.hpp>
#include "spell/definition_store.hpp"
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
