#include "fuzz_utils.hpp"
#include <algorithm>
#include <cstdlib>

namespace fuzz {

std::string dyslexic_misspell(const std::string& word, unsigned seed) {
  if (word.empty()) return word;
  std::minstd_rand rng(seed);
  std::string result = word;
  const std::string original = word;

  for (int attempt = 0; attempt < 15; ++attempt) {
    result = original;

    if (result.size() >= 2) {
      int op = rng() % 5;

      if (op == 0) {
        size_t pos = rng() % (result.size() - 1);
        std::swap(result[pos], result[pos + 1]);
      } else if (op == 1) {
        size_t pos = rng() % result.size();
        char c = result[pos];
        if (c == 'b') result[pos] = 'd';
        else if (c == 'd') result[pos] = 'b';
        else if (c == 'p') result[pos] = 'q';
        else if (c == 'q') result[pos] = 'p';
        else if (c == 'm') result[pos] = 'w';
        else if (c == 'w') result[pos] = 'm';
        else if (c == 'n') result[pos] = 'u';
        else if (c == 'u') result[pos] = 'n';
        else if (c == 'c') result[pos] = (rng() % 2) ? 'k' : 's';
        else if (c == 'k') result[pos] = 'c';
        else if (c == 's') result[pos] = 'c';
        else if (c == 'g') result[pos] = 'j';
        else if (c == 'j') result[pos] = 'g';
        else if (pos + 1 < result.size() && c == 'e' && result[pos + 1] == 'i') {
          result[pos] = 'i'; result[pos + 1] = 'e';
        } else if (pos + 1 < result.size() && c == 'i' && result[pos + 1] == 'e') {
          result[pos] = 'e'; result[pos + 1] = 'i';
        }
      } else if (op == 2) {
        size_t pos = rng() % result.size();
        result.insert(pos, 1, result[pos]);
      } else if (op == 3 && result.size() >= 3) {
        size_t pos = rng() % result.size();
        result.erase(pos, 1);
      } else if (op == 4 && result.size() >= 2) {
        for (size_t i = 0; i < result.size(); ++i) {
          if (i + 1 < result.size() && result[i] == 'p' && result[i + 1] == 'h') {
            result[i] = 'f'; result.erase(i + 1, 1); break;
          }
          if (result[i] == 'f') {
            result[i] = 'p'; result.insert(i + 1, 1, 'h'); break;
          }
          if (result[i] == 'c') { result[i] = 'k'; break; }
          if (result[i] == 'k') { result[i] = 'c'; break; }
        }
      }
    }

    if (result != original) return result;
  }
  return original;
}

std::string random_fuzz_word(std::minstd_rand& rng, size_t min_len, size_t max_len) {
  if (max_len < min_len) max_len = min_len;
  size_t len = min_len + (rng() % (max_len - min_len + 1));
  static const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  std::string result;
  result.reserve(len);
  for (size_t i = 0; i < len; ++i) {
    result += chars[rng() % (sizeof(chars) - 1)];
  }
  return result;
}

std::vector<std::string> generate_dyslexic_misspellings(
    const std::vector<std::string>& correct_words, size_t count, unsigned seed) {
  if (correct_words.empty()) return {};
  std::minstd_rand rng(seed);
  std::vector<std::string> result;
  result.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    const std::string& base = correct_words[rng() % correct_words.size()];
    std::string misspelled = dyslexic_misspell(base, static_cast<unsigned>(rng()));
    result.push_back(misspelled);
  }
  return result;
}

std::vector<std::string> generate_fuzz_words(size_t count, unsigned seed) {
  std::minstd_rand rng(seed);
  std::vector<std::string> result;
  result.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    result.push_back(random_fuzz_word(rng));
  }
  return result;
}

}  // namespace fuzz
