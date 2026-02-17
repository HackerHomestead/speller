#pragma once

#include <string>

namespace spell {

enum class RunMode { Interactive, Stream };

struct Config {
  RunMode mode = RunMode::Interactive;
  std::string file_path;       // For --file
  std::string dict_dir;        // For --dict-dir
  std::string defs_path;       // For --defs
  bool fast = false;           // --fast: auto-apply; --careful: prompt
  size_t max_suggestions = 5;  // Number of suggestions to show
  bool help_requested = false; // --help or -h

  static Config from_args(int argc, char* argv[]);
};

}  // namespace spell
