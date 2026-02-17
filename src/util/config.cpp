#include "util/config.hpp"
#include <cstdlib>
#include <cstring>
#include <fstream>

namespace spell {

namespace {

/// Return path to first existing config file, or preferred path if none exist.
std::string get_config_path() {
  const char* home = std::getenv("HOME");
  if (!home || !*home) return "";
  std::string base(home);
  std::string path = base + "/.config/spell/config";
  std::ifstream f(path);
  if (f.good()) return path;
  path = base + "/.spellrc";
  f.open(path);
  if (f.good()) return path;
  return base + "/.config/spell/config";
}

void load_config_file(Config& cfg, const std::string& path) {
  std::ifstream f(path);
  if (!f) return;
  std::string line;
  while (std::getline(f, line)) {
    size_t hash = line.find('#');
    if (hash != std::string::npos) line = line.substr(0, hash);
    size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string key = line.substr(0, eq);
    std::string val = line.substr(eq + 1);
    auto trim = [](std::string& s) {
      while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(0, 1);
      while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
    };
    trim(key);
    trim(val);
    if (key == "dict_dir" && !val.empty()) cfg.dict_dir = val;
    else if (key == "user_dict" && !val.empty()) cfg.user_dict_path = val;
    else if (key == "max_suggestions" && !val.empty()) {
      try { cfg.max_suggestions = static_cast<size_t>(std::stoul(val)); } catch (...) {}
    }
  }
}

}  // namespace

Config Config::from_args(int argc, char* argv[]) {
  Config cfg;

  std::string config_path = get_config_path();
  if (!config_path.empty()) {
    std::ifstream probe(config_path);
    if (probe.good()) {
      load_config_file(cfg, config_path);
      cfg.config_file_path = config_path;
    }
  }

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
    } else if (std::strcmp(arg, "--version") == 0 || std::strcmp(arg, "-V") == 0) {
      cfg.version_requested = true;
    }
  }

  return cfg;
}

}  // namespace spell
