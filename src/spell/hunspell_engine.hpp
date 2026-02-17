#pragma once

#include "spell/spell_engine.hpp"
#include <memory>
#include <string>

namespace spell {

#ifdef SPELL_HAS_HUNSPELL
class Hunspell;
#endif

/// SpellEngine implementation using Hunspell. Loads .aff and .dic from dict_dir.
/// When Hunspell is not available, all methods return false/empty.
class HunspellEngine : public SpellEngine {
 public:
  /// Create engine. dict_dir should contain e.g. en_US.aff and en_US.dic.
  /// base_name: e.g. "en_US" (without .aff/.dic).
  /// user_dict_path: optional path to user dictionary (one word per line).
  static std::unique_ptr<HunspellEngine> create(const std::string& dict_dir,
                                                const std::string& base_name = "en_US",
                                                const std::string& user_dict_path = "");

  ~HunspellEngine() override;

  HunspellEngine(HunspellEngine&&) = default;
  HunspellEngine& operator=(HunspellEngine&&) = default;
  HunspellEngine(const HunspellEngine&) = delete;
  HunspellEngine& operator=(const HunspellEngine&) = delete;

  bool is_correct(const std::string& word) override;
  std::vector<Suggestion> suggest(const std::string& word) override;

  /// True if the engine loaded successfully and can check words.
  bool is_loaded() const { return loaded_; }

  HunspellEngine() = default;

 private:

#ifdef SPELL_HAS_HUNSPELL
  std::unique_ptr<Hunspell> hunspell_;
#endif
  bool loaded_ = false;
};

}  // namespace spell
