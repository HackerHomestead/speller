#include "util/config.hpp"
#include <iostream>
#include <cstdlib>

namespace {

const char* help_text =
    "spell - Offline spell checker and definition tool\n"
    "\n"
    "Usage: spell [options]\n"
    "\n"
    "Options:\n"
    "  --interactive    Interactive word-by-word mode (default)\n"
    "  --stream         Stream mode: read from stdin/file, write to stdout\n"
    "  --file PATH      Read from file (implies --stream)\n"
    "  --dict-dir PATH  Dictionary directory (.aff, .dic files)\n"
    "  --defs PATH      Definition database path\n"
    "  --fast           Auto-apply top suggestion in stream mode\n"
    "  --careful        Prompt when confidence is low (default)\n"
    "  -h, --help       Show this help\n";

int run_spell(const spell::Config& config) {
  (void)config;
  std::cout << "Spell module (stub)\n";
  return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
  auto config = spell::Config::from_args(argc, argv);

  if (config.help_requested) {
    std::cout << help_text;
    return 0;
  }

  return run_spell(config);
}
