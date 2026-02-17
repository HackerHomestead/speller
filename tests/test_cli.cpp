#include <catch2/catch_test_macros.hpp>
#include "util/config.hpp"

using namespace spell;

TEST_CASE("Config --help sets help_requested", "[cli]") {
  const char* argv[] = {"spell", "--help"};
  auto cfg = Config::from_args(2, const_cast<char**>(argv));
  REQUIRE(cfg.help_requested);
}

TEST_CASE("Config -h sets help_requested", "[cli]") {
  const char* argv[] = {"spell", "-h"};
  auto cfg = Config::from_args(2, const_cast<char**>(argv));
  REQUIRE(cfg.help_requested);
}
