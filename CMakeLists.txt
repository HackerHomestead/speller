cmake_minimum_required(VERSION 3.14)
project(spell VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Hunspell (optional for Phase 1; required for Phase 2)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(HUNSPELL QUIET hunspell)
endif()

# Catch2 for tests
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Spell library (interfaces + implementations)
add_library(spell_lib
  src/spell/stub_spell_engine.cpp
  src/spell/stub_definition_store.cpp
  src/spell/suggestion_orchestrator.cpp
  src/util/config.cpp
)
target_include_directories(spell_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
if(HUNSPELL_FOUND)
  target_include_directories(spell_lib PRIVATE ${HUNSPELL_INCLUDE_DIRS})
  target_link_libraries(spell_lib PRIVATE ${HUNSPELL_LIBRARIES})
  target_compile_definitions(spell_lib PRIVATE SPELL_HAS_HUNSPELL=1)
endif()

# Main executable
add_executable(spell src/main.cpp)
target_link_libraries(spell PRIVATE spell_lib)

# Tests
add_executable(spell_tests
  tests/test_spell_engine.cpp
  tests/test_definition_store.cpp
  tests/test_suggestion_orchestrator.cpp
  tests/test_config.cpp
  tests/test_cli.cpp
)
target_link_libraries(spell_tests PRIVATE spell_lib Catch2::Catch2WithMain)
target_include_directories(spell_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(CTest)
enable_testing()
add_test(NAME spell_tests COMMAND spell_tests)
