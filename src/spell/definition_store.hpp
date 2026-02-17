#pragma once

#include <string>

namespace spell {

struct Definition {
  std::string headword;
  std::string part_of_speech;
  std::string short_gloss;

  bool empty() const { return headword.empty(); }
};

class DefinitionStore {
 public:
  virtual ~DefinitionStore() = default;
  virtual Definition lookup(const std::string& word) = 0;
};

}  // namespace spell
