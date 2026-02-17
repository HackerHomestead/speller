#pragma once

#include <memory>
#include <string>
#include <vector>

namespace spell {

struct Suggestion {
  std::string word;
  float score;  // Higher = better
};

class SpellEngine {
 public:
  virtual ~SpellEngine() = default;

  virtual bool is_correct(const std::string& word) = 0;
  virtual std::vector<Suggestion> suggest(const std::string& word) = 0;
};

}  // namespace spell
