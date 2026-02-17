#pragma once

#include "spell/definition_store.hpp"
#include <map>
#include <memory>
#include <string>

namespace spell {

/// DefinitionStore that loads from a text file.
/// Format: one definition per line: "word<TAB>pos<TAB>gloss" or "word<TAB>gloss".
/// Lines starting with # are comments. Lookup is case-insensitive.
class FileDefinitionStore : public DefinitionStore {
 public:
  /// Load from path. Returns null on failure; otherwise a ready-to-use store.
  static std::unique_ptr<FileDefinitionStore> load(const std::string& path);

  Definition lookup(const std::string& word) override;

 private:
  FileDefinitionStore() = default;
  std::map<std::string, Definition> definitions_;
};

}  // namespace spell
