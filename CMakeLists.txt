cmake_minimum_required(VERSION 3.14)
project(spell VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# Dependency checking
# -----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "spell ${PROJECT_VERSION} - Dependency check")
message(STATUS "----------------------------------------")

# Required: C++ compiler (implicit)
message(STATUS "  C++ compiler:    ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# Required: CMake
message(STATUS "  CMake:            ${CMAKE_VERSION} (min 3.14)")

# Optional: Hunspell
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(HUNSPELL QUIET hunspell)
endif()
if(NOT HUNSPELL_FOUND)
  find_path(HUNSPELL_INCLUDE_DIR NAMES hunspell/hunspell.hxx)
  find_library(HUNSPELL_LIBRARY NAMES hunspell)
  if(HUNSPELL_INCLUDE_DIR AND HUNSPELL_LIBRARY)
    set(HUNSPELL_FOUND TRUE)
    set(HUNSPELL_INCLUDE_DIRS ${HUNSPELL_INCLUDE_DIR})
    set(HUNSPELL_LIBRARIES ${HUNSPELL_LIBRARY})
  endif()
endif()
if(HUNSPELL_FOUND)
  message(STATUS "  Hunspell:         YES (${HUNSPELL_LIBRARIES})")
  set(SPELL_HUNSPELL_STATUS "enabled")
else()
  message(STATUS "  Hunspell:         NO  (optional - install libhunspell-dev)")
  set(SPELL_HUNSPELL_STATUS "disabled")
endif()

# Optional: readline (REPL autocomplete, history)
if(PkgConfig_FOUND)
  pkg_check_modules(READLINE QUIET readline)
endif()
if(NOT READLINE_FOUND)
  find_library(READLINE_LIBRARY NAMES readline)
  if(READLINE_LIBRARY)
    set(READLINE_FOUND TRUE)
    set(READLINE_LIBRARIES ${READLINE_LIBRARY})
  endif()
endif()
if(READLINE_FOUND)
  message(STATUS "  Readline:         YES (autocomplete, history)")
else()
  message(STATUS "  Readline:         NO  (optional - install libreadline-dev)")
endif()

# Catch2 (FetchContent - always available)
message(STATUS "  Catch2:           fetched automatically (tests)")

message(STATUS "----------------------------------------")
message(STATUS "  Spell checking:   ${SPELL_HUNSPELL_STATUS}")
message(STATUS "")

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
  src/spell/hunspell_engine.cpp
  src/spell/suggestion_orchestrator.cpp
  src/spell/repl.cpp
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
if(READLINE_FOUND)
  target_link_libraries(spell_lib PRIVATE ${READLINE_LIBRARIES})
  target_compile_definitions(spell_lib PRIVATE SPELL_HAS_READLINE=1)
endif()

# Main executable
add_executable(spell src/main.cpp)
target_link_libraries(spell PRIVATE spell_lib)
target_compile_definitions(spell PRIVATE SPELL_VERSION="${PROJECT_VERSION}")
if(HUNSPELL_FOUND)
  target_compile_definitions(spell PRIVATE SPELL_HAS_HUNSPELL=1)
endif()

# Tests
add_executable(spell_tests
  tests/test_spell_engine.cpp
  tests/test_definition_store.cpp
  tests/test_suggestion_orchestrator.cpp
  tests/test_config.cpp
  tests/test_cli.cpp
  tests/test_hunspell_engine.cpp
  tests/test_functional.cpp
  tests/test_fuzz.cpp
  tests/fuzz_utils.cpp
)
target_link_libraries(spell_tests PRIVATE spell_lib Catch2::Catch2WithMain)
target_include_directories(spell_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/tests
)
target_compile_definitions(spell_tests PRIVATE
  SPELL_DICT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/dict"
  SPELL_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
  SPELL_BINARY="${CMAKE_CURRENT_BINARY_DIR}/spell")
if(HUNSPELL_FOUND)
  target_compile_definitions(spell_tests PRIVATE SPELL_HAS_HUNSPELL=1)
endif()

include(CTest)
enable_testing()
add_test(NAME spell_tests COMMAND spell_tests WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
