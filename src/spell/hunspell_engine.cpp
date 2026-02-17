#include "spell/hunspell_engine.hpp"
#include <fstream>
#include <sstream>

#ifdef SPELL_HAS_HUNSPELL
#include <hunspell/hunspell.hxx>
#endif

namespace spell {

std::unique_ptr<HunspellEngine> HunspellEngine::create(const std::string& dict_dir,
                                                      const std::string& base_name,
                                                      const std::string& user_dict_path) {
  auto engine = std::make_unique<HunspellEngine>();

#ifdef SPELL_HAS_HUNSPELL
  std::string aff_path = dict_dir;
  if (!aff_path.empty() && aff_path.back() != '/') aff_path += '/';
  std::string dic_path = aff_path + base_name + ".dic";
  aff_path += base_name + ".aff";

  std::ifstream aff_file(aff_path);
  std::ifstream dic_file(dic_path);
  if (!aff_file || !dic_file) {
    return engine;  // loaded_ stays false
  }

  try {
    engine->hunspell_ = std::make_unique<Hunspell>(aff_path.c_str(), dic_path.c_str());

    // Load user dictionary
    if (!user_dict_path.empty()) {
      std::ifstream user_file(user_dict_path);
      if (user_file) {
        std::string word;
        while (std::getline(user_file, word)) {
          // Trim and skip empty
          while (!word.empty() && (word.back() == '\r' || word.back() == ' ')) word.pop_back();
          if (!word.empty() && word[0] != '#') {
            engine->hunspell_->add(word);
          }
        }
      }
    }

    engine->loaded_ = true;
  } catch (...) {
    // Hunspell constructor can throw
  }
#endif

  return engine;
}

HunspellEngine::~HunspellEngine() = default;

bool HunspellEngine::is_correct(const std::string& word) {
#ifdef SPELL_HAS_HUNSPELL
  if (loaded_ && hunspell_) {
    return hunspell_->spell(word);
  }
#endif
  return false;
}

std::vector<Suggestion> HunspellEngine::suggest(const std::string& word) {
  std::vector<Suggestion> result;
#ifdef SPELL_HAS_HUNSPELL
  if (loaded_ && hunspell_) {
    auto suggestions = hunspell_->suggest(word);
    float score = 1.0f;
    for (const auto& s : suggestions) {
      result.push_back({s, score});
      score -= 0.1f;  // Decreasing score for lower-ranked suggestions
    }
  }
#endif
  return result;
}

}  // namespace spell
