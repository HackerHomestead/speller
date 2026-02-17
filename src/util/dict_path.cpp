#include "util/dict_path.hpp"
#include <algorithm>

namespace spell {

std::pair<std::string, std::string> parse_dict_path(std::string path) {
  while (!path.empty() && (path.back() == ' ' || path.back() == '\t')) path.pop_back();
  if (path.empty()) return {"", "en_US"};
  const std::string aff = ".aff";
  if (path.size() >= aff.size() &&
      path.compare(path.size() - aff.size(), aff.size(), aff) == 0) {
    std::string base = path.substr(0, path.size() - aff.size());
    size_t slash = base.find_last_of("/\\");
    std::string dir = (slash != std::string::npos) ? base.substr(0, slash) : ".";
    std::string name = (slash != std::string::npos) ? base.substr(slash + 1) : base;
    if (name.empty()) name = "en_US";
    return {dir, name};
  }
  return {path, "en_US"};
}

}  // namespace spell
