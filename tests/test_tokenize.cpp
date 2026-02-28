#include <catch2/catch_test_macros.hpp>
#include "spell/tokenize.hpp"
#include <vector>
#include <string>

using namespace spell;

TEST_CASE("Tokenize sentence - basic words", "[tokenize]") {
  std::string input = "hello world";
  auto tokens = tokenize_sentence(input);
  
  REQUIRE(tokens.size() == 3);
  REQUIRE(tokens[0].text == "hello");
  REQUIRE(tokens[0].original == "hello");
  REQUIRE(tokens[0].is_word == true);
  REQUIRE(tokens[1].text == " ");
  REQUIRE(tokens[1].is_space == true);
  REQUIRE(tokens[2].text == "world");
  REQUIRE(tokens[2].original == "world");
  REQUIRE(tokens[2].is_word == true);
}

TEST_CASE("Tokenize sentence - with punctuation", "[tokenize]") {
  std::string input = "hello, world!";
  auto tokens = tokenize_sentence(input);
  
  REQUIRE(tokens.size() == 5);
  REQUIRE(tokens[0].text == "hello");
  REQUIRE(tokens[1].text == ",");
  REQUIRE(tokens[1].is_word == false);
  REQUIRE(tokens[2].text == " ");
  REQUIRE(tokens[2].is_space == true);
  REQUIRE(tokens[3].text == "world");
  REQUIRE(tokens[4].text == "!");
}

TEST_CASE("Tokenize sentence - with spaces", "[tokenize]") {
  std::string input = "hello  world";
  auto tokens = tokenize_sentence(input);
  
  REQUIRE(tokens.size() == 4);
  REQUIRE(tokens[0].text == "hello");
  REQUIRE(tokens[1].text == " ");
  REQUIRE(tokens[1].is_space == true);
  REQUIRE(tokens[2].text == " ");
  REQUIRE(tokens[2].is_space == true);
  REQUIRE(tokens[3].text == "world");
}

TEST_CASE("Tokenize sentence - empty", "[tokenize]") {
  std::string input = "";
  auto tokens = tokenize_sentence(input);
  REQUIRE(tokens.empty());
}

TEST_CASE("Split into words - basic", "[split_words]") {
  std::string input = "hello world";
  auto words = split_into_words(input);
  
  REQUIRE(words.size() == 2);
  REQUIRE(words[0] == "hello");
  REQUIRE(words[1] == "world");
}

TEST_CASE("Split into words - multiple spaces", "[split_words]") {
  std::string input = "hello   world test";
  auto words = split_into_words(input);
  
  REQUIRE(words.size() == 3);
  REQUIRE(words[0] == "hello");
  REQUIRE(words[1] == "world");
  REQUIRE(words[2] == "test");
}

TEST_CASE("Split into words - punctuation", "[split_words]") {
  std::string input = "hello world!test";
  auto words = split_into_words(input);
  
  REQUIRE(words.size() == 2);
  REQUIRE(words[0] == "hello");
  REQUIRE(words[1] == "world!test");
}

TEST_CASE("Split into words - empty", "[split_words]") {
  std::string input = "";
  auto words = split_into_words(input);
  REQUIRE(words.empty());
}
