#include "util/term_format.hpp"
#include <cstdlib>
#include <cstring>
#include <unistd.h>

namespace spell {

namespace {

const char* const BOLD = "\033[1m";
const char* const DIM = "\033[2m";
const char* const CYAN = "\033[36m";
const char* const GREEN = "\033[32m";
const char* const YELLOW = "\033[33m";
const char* const RESET = "\033[0m";

}  // namespace

bool term_use_color(std::ostream& out) {
  if (std::getenv("NO_COLOR") != nullptr && std::getenv("NO_COLOR")[0] != '\0')
    return false;
  if (&out == &std::cout && isatty(STDOUT_FILENO))
    return true;
  return false;
}

void term_bold(std::ostream& out, const std::string& word) {
  if (term_use_color(out))
    out << BOLD << word << RESET;
  else
    out << word;
}

void term_bold_yellow(std::ostream& out, const std::string& word) {
  if (term_use_color(out))
    out << BOLD << YELLOW << word << RESET;
  else
    out << word;
}

void term_print_definition(std::ostream& out, const Definition& def) {
  if (def.empty()) return;
  out << "  ";
  term_bold(out, def.headword);
  if (!def.part_of_speech.empty())
    out << " (" << def.part_of_speech << ") ";
  else
    out << " ";
  if (term_use_color(out))
    out << DIM << CYAN << def.short_gloss << RESET;
  else
    out << def.short_gloss;
  out << "\n";
}

void term_print_definition_inline(std::ostream& out, const std::string& word, const Definition& def) {
  if (def.empty()) return;
  out << "  ";
  term_bold(out, word);
  if (!def.part_of_speech.empty())
    out << " (" << def.part_of_speech << ") ";
  else
    out << " ";
  if (term_use_color(out))
    out << DIM << CYAN << def.short_gloss << RESET;
  else
    out << def.short_gloss;
  out << "\n";
}

}  // namespace spell
