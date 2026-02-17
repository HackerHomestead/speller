# Build Hunspell as a static library from fetched source.
# Expects HUNSPELL_SOURCE_DIR to point at the root of the hunspell repo (e.g. from FetchContent).

set(HUNSPELL_SRC_DIR "${HUNSPELL_SOURCE_DIR}/src/hunspell")

# Visibility header for static build (no exports)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/hunvisapi.h"
  "#ifndef HUNSPELL_VISIBILITY_H_\n"
  "#define HUNSPELL_VISIBILITY_H_\n"
  "#define LIBHUNSPELL_DLL_EXPORTED\n"
  "#endif\n")

add_library(hunspell STATIC
  ${HUNSPELL_SRC_DIR}/affentry.cxx
  ${HUNSPELL_SRC_DIR}/affixmgr.cxx
  ${HUNSPELL_SRC_DIR}/csutil.cxx
  ${HUNSPELL_SRC_DIR}/filemgr.cxx
  ${HUNSPELL_SRC_DIR}/hashmgr.cxx
  ${HUNSPELL_SRC_DIR}/hunspell.cxx
  ${HUNSPELL_SRC_DIR}/hunzip.cxx
  ${HUNSPELL_SRC_DIR}/phonet.cxx
  ${HUNSPELL_SRC_DIR}/replist.cxx
  ${HUNSPELL_SRC_DIR}/suggestmgr.cxx
)

# Public: so that #include <hunspell/hunspell.hxx> resolves (src/hunspell/hunspell.hxx)
target_include_directories(hunspell
  PUBLIC
    ${HUNSPELL_SOURCE_DIR}/src
  PRIVATE
    ${HUNSPELL_SOURCE_DIR}/src/hunspell
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(hunspell PRIVATE
  BUILDING_LIBHUNSPELL
  HUNSPELL_STATIC
)

# C++11 is enough for Hunspell; we build with project's C++17
target_compile_features(hunspell PUBLIC cxx_std_11)

# Optional: iconv (e.g. on musl or non-glibc); glibc has iconv built-in
find_library(ICONV_LIBRARY NAMES iconv iconv.lib)
if(ICONV_LIBRARY)
  target_link_libraries(hunspell PRIVATE ${ICONV_LIBRARY})
endif()

add_library(hunspell::hunspell ALIAS hunspell)
