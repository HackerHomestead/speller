#pragma once

#include <string>
#include <utility>

namespace spell {

/// Parse a dictionary path: directory or path to .aff file.
/// If path ends with .aff, returns (parent_dir, base_name) e.g. ("/data/dict", "en_US").
/// Otherwise returns (path, "en_US").
std::pair<std::string, std::string> parse_dict_path(std::string path);

}  // namespace spell
