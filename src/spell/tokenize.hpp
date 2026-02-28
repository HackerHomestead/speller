#pragma once

#include <string>
#include <vector>

namespace spell {

struct Token {
  std::string text;
  std::string original;
  bool is_word;
  bool is_space;
};

std::vector<Token> tokenize_sentence(const std::string& sentence);

std::vector<std::string> split_into_words(const std::string& s);

}  // namespace spell
