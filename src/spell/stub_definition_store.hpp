#pragma once

#include "spell/definition_store.hpp"
#include <map>

namespace spell {

/// Stub implementation for testing. Lookup returns preconfigured definitions.
class StubDefinitionStore : public DefinitionStore {
 public:
  void add_definition(const std::string& word, const std::string& pos,
                     const std::string& gloss);

  Definition lookup(const std::string& word) override;

 private:
  std::map<std::string, Definition> definitions_;
};

}  // namespace spell
