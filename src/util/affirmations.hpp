#pragma once

#include <cstdlib>
#include <ctime>

namespace spell {

/// Return a random positive affirmation for correct spelling (not "OK", which is for system ops).
inline const char* random_correct_affirmation() {
  static bool seeded = false;
  if (!seeded) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    seeded = true;
  }
  static const char* const messages[] = {
    "Correct!",
    "That's correct!",
    "Right!",
    "Spelled correctly!",
    "Yes!",
    "Good spelling!",
    "Well spelled!",
    "Perfect!",
    "You got it!",
    "Nice!",
  };
  enum { N = sizeof(messages) / sizeof(messages[0]) };
  return messages[static_cast<unsigned>(std::rand()) % N];
}

}  // namespace spell
