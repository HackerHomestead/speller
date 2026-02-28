#include "spell/tokenize.hpp"
#include <cctype>

namespace spell {

bool is_word_char(char c) {
  return std::isalpha(static_cast<unsigned char>(c));
}

std::vector<Token> tokenize_sentence(const std::string& sentence) {
  std::vector<Token> tokens;
  std::string current;
  
  for (size_t i = 0; i < sentence.size(); ++i) {
    char c = sentence[i];
    if (is_word_char(c)) {
      current += c;
    } else if (std::isspace(static_cast<unsigned char>(c))) {
      if (!current.empty()) {
        Token t;
        t.text = current;
        t.original = current;
        t.is_word = true;
        t.is_space = false;
        tokens.push_back(t);
        current.clear();
      }
      Token t;
      t.text = " ";
      t.original = " ";
      t.is_word = false;
      t.is_space = true;
      tokens.push_back(t);
    } else {
      if (!current.empty()) {
        Token t;
        t.text = current;
        t.original = current;
        t.is_word = true;
        t.is_space = false;
        tokens.push_back(t);
        current.clear();
      }
      Token t;
      t.text = std::string(1, c);
      t.original = t.text;
      t.is_word = false;
      t.is_space = false;
      tokens.push_back(t);
    }
  }
  
  if (!current.empty()) {
    Token t;
    t.text = current;
    t.original = current;
    t.is_word = true;
    t.is_space = false;
    tokens.push_back(t);
  }
  
  return tokens;
}

std::vector<std::string> split_into_words(const std::string& s) {
  std::vector<std::string> words;
  std::string current;
  for (size_t i = 0; i < s.size(); ++i) {
    char c = s[i];
    if (std::isspace(static_cast<unsigned char>(c))) {
      if (!current.empty()) {
        words.push_back(current);
        current.clear();
      }
    } else {
      current += c;
    }
  }
  if (!current.empty()) {
    words.push_back(current);
  }
  return words;
}

}  // namespace spell
