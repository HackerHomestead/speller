#pragma once

#include "spell/definition_store.hpp"
#include <iostream>
#include <string>

namespace spell {

/// Whether to use ANSI colors (false if NO_COLOR is set or not a TTY).
bool term_use_color(std::ostream& out);

/// Write word in bold (ANSI) to out.
void term_bold(std::ostream& out, const std::string& word);

/// Write word in bold+yellow (ANSI). Use for suggested words that have a definition.
void term_bold_yellow(std::ostream& out, const std::string& word);

/// Write a definition line: "  word (pos.) gloss" with bold word and colored gloss.
void term_print_definition(std::ostream& out, const Definition& def);

/// Write a one-line definition for correct-spelling case: "  word (pos.) gloss" or just "  gloss".
void term_print_definition_inline(std::ostream& out, const std::string& word, const Definition& def);

}  // namespace spell
