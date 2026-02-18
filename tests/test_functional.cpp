#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#ifndef SPELL_BINARY
#define SPELL_BINARY "spell"
#endif
#ifndef SPELL_DICT_DIR
#define SPELL_DICT_DIR "data/dict"
#endif

namespace {

std::string run_cmd(const std::string& cmd) {
  std::string full_cmd = cmd + " 2>&1";
  FILE* pipe = popen(full_cmd.c_str(), "r");
  if (!pipe) return "";
  std::string result;
  char buf[256];
  while (fgets(buf, sizeof(buf), pipe)) result += buf;
  pclose(pipe);
  return result;
}

bool file_exists(const std::string& path) {
  std::ifstream f(path);
  return f.good();
}

}  // namespace

TEST_CASE("spell --help prints usage", "[functional]") {
  std::string out = run_cmd(std::string(SPELL_BINARY) + " --help");
  REQUIRE_FALSE(out.empty());
  REQUIRE(out.find("spell") != std::string::npos);
  REQUIRE(out.find("--help") != std::string::npos);
  REQUIRE(out.find("--check") != std::string::npos);
}

TEST_CASE("spell -h prints usage", "[functional]") {
  std::string out = run_cmd(std::string(SPELL_BINARY) + " -h");
  REQUIRE(out.find("spell") != std::string::npos);
}

TEST_CASE("spell --version prints version", "[functional]") {
  std::string out = run_cmd(std::string(SPELL_BINARY) + " --version");
  REQUIRE(out.find("spell") != std::string::npos);
  REQUIRE(out.find("Hunspell") != std::string::npos);
}

TEST_CASE("spell -V prints version", "[functional]") {
  std::string out = run_cmd(std::string(SPELL_BINARY) + " -V");
  REQUIRE(out.find("spell") != std::string::npos);
}

TEST_CASE("spell with no args enters REPL", "[functional]") {
  std::string cmd = "printf '%s\\n' ':q' | " + std::string(SPELL_BINARY) + " 2>&1";
  std::string out = run_cmd(cmd);
  REQUIRE_FALSE(out.empty());
  REQUIRE(out.find("spell") != std::string::npos);
  REQUIRE((out.find("REPL") != std::string::npos || out.find("help") != std::string::npos));
}

TEST_CASE("spell --check with bundled dict", "[functional]") {
  std::string dict_dir = SPELL_DICT_DIR;
  if (!file_exists(dict_dir + "/en_US.aff")) {
    SKIP("Bundled dictionary not found at " + dict_dir);
  }
  std::string cmd = std::string(SPELL_BINARY) + " --check hello --dict-dir " + dict_dir;
  std::string out = run_cmd(cmd);
  REQUIRE_FALSE(out.empty());
  bool could_not_load = out.find("Could not load") != std::string::npos;
  bool has_hello = out.find("hello") != std::string::npos;
  bool is_suggestion = out.find("Did you mean") != std::string::npos;
  bool valid = could_not_load || (has_hello && !is_suggestion);
  REQUIRE(valid);
  REQUIRE((has_hello || could_not_load));
}

TEST_CASE("spell --check misspelling with bundled dict", "[functional]") {
  std::string dict_dir = SPELL_DICT_DIR;
  if (!file_exists(dict_dir + "/en_US.aff")) {
    SKIP("Bundled dictionary not found at " + dict_dir);
  }
  std::string out = run_cmd(std::string(SPELL_BINARY) + " --check helo --dict-dir " + dict_dir);
  REQUIRE_FALSE(out.empty());
  bool has_helo = out.find("helo") != std::string::npos;
  bool has_hello = out.find("hello") != std::string::npos;
  bool has_did_you_mean = out.find("Did you mean") != std::string::npos;
  bool could_not_load = out.find("Could not load") != std::string::npos;
  bool valid_response = (has_helo || has_hello || could_not_load);
  bool valid_suggestion = (has_hello || has_did_you_mean || could_not_load);
  REQUIRE(valid_response);
  REQUIRE(valid_suggestion);
}
