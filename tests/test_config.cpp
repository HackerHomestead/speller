#include <catch2/catch_test_macros.hpp>
#include "util/config.hpp"

using namespace spell;

TEST_CASE("Config defaults to Interactive mode", "[config]") {
  const char* argv[] = {"spell"};
  auto cfg = Config::from_args(1, const_cast<char**>(argv));
  REQUIRE(cfg.mode == RunMode::Interactive);
  REQUIRE(cfg.max_suggestions == 5);
}

TEST_CASE("Config --stream sets Stream mode", "[config]") {
  const char* argv[] = {"spell", "--stream"};
  auto cfg = Config::from_args(2, const_cast<char**>(argv));
  REQUIRE(cfg.mode == RunMode::Stream);
}

TEST_CASE("Config --file sets path and Stream mode", "[config]") {
  const char* argv[] = {"spell", "--file", "/tmp/test.txt"};
  auto cfg = Config::from_args(3, const_cast<char**>(argv));
  REQUIRE(cfg.mode == RunMode::Stream);
  REQUIRE(cfg.file_path == "/tmp/test.txt");
}

TEST_CASE("Config --dict-dir sets dict_dir", "[config]") {
  const char* argv[] = {"spell", "--dict-dir", "/opt/dict"};
  auto cfg = Config::from_args(3, const_cast<char**>(argv));
  REQUIRE(cfg.dict_dir == "/opt/dict");
}

TEST_CASE("Config --fast sets fast mode", "[config]") {
  const char* argv[] = {"spell", "--fast"};
  auto cfg = Config::from_args(2, const_cast<char**>(argv));
  REQUIRE(cfg.fast == true);
}
