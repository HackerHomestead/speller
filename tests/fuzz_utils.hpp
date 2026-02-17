#pragma once

#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <cstddef>

namespace fuzz {

/// Generate dyslexic-like misspellings from a correct word.
/// Uses: letter reversals, transposition, doubling, omission.
std::string dyslexic_misspell(const std::string& word, unsigned seed);

/// Generate random fuzz strings (nonsense, not based on real words).
std::string random_fuzz_word(std::minstd_rand& rng, size_t min_len = 2, size_t max_len = 12);

/// Generate N dyslexic misspellings from a word list.
std::vector<std::string> generate_dyslexic_misspellings(
    const std::vector<std::string>& correct_words, size_t count, unsigned seed);

/// Generate N random fuzz words.
std::vector<std::string> generate_fuzz_words(size_t count, unsigned seed);

}  // namespace fuzz
