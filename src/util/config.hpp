#pragma once

#include <string>

namespace spell {

enum class RunMode { Interactive, Stream };

struct Config {
  RunMode mode = RunMode::Interactive;
  std::string file_path;       // For --file
  std::string check_word;      // For --check (single word check)
  std::string dict_dir;        // For --dict-dir
  std::string user_dict_path;  // For user dictionary (e.g. user.dic)
  std::string defs_path;       // For --defs
  bool fast = false;           // --fast: auto-apply; --careful: prompt
  size_t max_suggestions = 5;  // Number of suggestions to show
  bool help_requested = false;  // --help or -h
  bool version_requested = false;  // --version or -V

  /// Load defaults from config file, then override with argv.
  static Config from_args(int argc, char* argv[]);

  /// Path to config file (for display). Empty if none loaded.
  std::string config_file_path;
};

}  // namespace spell
