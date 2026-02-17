#include "spell/stub_definition_store.hpp"

namespace spell {

void StubDefinitionStore::add_definition(const std::string& word,
                                         const std::string& pos,
                                         const std::string& gloss) {
  Definition def;
  def.headword = word;
  def.part_of_speech = pos;
  def.short_gloss = gloss;
  definitions_[word] = std::move(def);
}

Definition StubDefinitionStore::lookup(const std::string& word) {
  auto it = definitions_.find(word);
  if (it == definitions_.end()) return {};
  return it->second;
}

}  // namespace spell
