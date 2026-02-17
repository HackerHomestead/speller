#include "util/config.hpp"
#include <cstring>

namespace spell {

Config Config::from_args(int argc, char* argv[]) {
  Config cfg;

  for (int i = 1; i < argc; ++i) {
    const char* arg = argv[i];
    if (std::strcmp(arg, "--interactive") == 0) {
      cfg.mode = RunMode::Interactive;
    } else if (std::strcmp(arg, "--stream") == 0) {
      cfg.mode = RunMode::Stream;
    } else if (std::strcmp(arg, "--check") == 0 && i + 1 < argc) {
      cfg.check_word = argv[++i];
    } else if (std::strcmp(arg, "--file") == 0 && i + 1 < argc) {
      cfg.file_path = argv[++i];
      cfg.mode = RunMode::Stream;
    } else if (std::strcmp(arg, "--dict-dir") == 0 && i + 1 < argc) {
      cfg.dict_dir = argv[++i];
    } else if (std::strcmp(arg, "--user-dict") == 0 && i + 1 < argc) {
      cfg.user_dict_path = argv[++i];
    } else if (std::strcmp(arg, "--defs") == 0 && i + 1 < argc) {
      cfg.defs_path = argv[++i];
    } else if (std::strcmp(arg, "--fast") == 0) {
      cfg.fast = true;
    } else if (std::strcmp(arg, "--careful") == 0) {
      cfg.fast = false;
    } else if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0) {
      cfg.help_requested = true;
    }
  }

  return cfg;
}

}  // namespace spell
