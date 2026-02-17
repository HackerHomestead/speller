#include "spell/file_definition_store.hpp"
#include <cctype>
#include <fstream>

namespace spell {

namespace {

std::string to_lower(const std::string& s) {
  std::string r = s;
  for (char& c : r)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return r;
}

}  // namespace

std::unique_ptr<FileDefinitionStore> FileDefinitionStore::load(const std::string& path) {
  std::ifstream f(path);
  if (!f) return nullptr;
  auto store = std::unique_ptr<FileDefinitionStore>(new FileDefinitionStore());
  std::string line;
  while (std::getline(f, line)) {
    if (line.empty() || line[0] == '#') continue;
    size_t t1 = line.find('\t');
    if (t1 == std::string::npos) continue;
    std::string word = line.substr(0, t1);
    size_t t2 = line.find('\t', t1 + 1);
    std::string pos, gloss;
    if (t2 != std::string::npos) {
      pos = line.substr(t1 + 1, t2 - (t1 + 1));
      gloss = line.substr(t2 + 1);
    } else {
      gloss = line.substr(t1 + 1);
    }
    Definition def;
    def.headword = word;
    def.part_of_speech = pos;
    def.short_gloss = gloss;
    store->definitions_[to_lower(word)] = std::move(def);
  }
  return store;
}

Definition FileDefinitionStore::lookup(const std::string& word) {
  auto it = definitions_.find(to_lower(word));
  if (it == definitions_.end()) return {};
  return it->second;
}

}  // namespace spell
