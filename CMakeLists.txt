cmake_minimum_required (VERSION 3.1)

project (specgram VERSION 1.0.0 LANGUAGES CXX)

# Version
set (VERSION_MAJOR 0)
set (VERSION_MINOR 9)
set (VERSION_PATCH 0)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package (spdlog REQUIRED)
find_package (SFML 2.5 COMPONENTS window graphics REQUIRED)
find_library (FFTW3 fftw3)

# Compiler setup
add_compile_options (-Wall -Wextra -pedantic)

# Directory setup
set (SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set (MAN_DIR "${PROJECT_SOURCE_DIR}/man")

include_directories (${SOURCE_DIR})

# Get the latest abbreviated commit hash of the working branch
# https://jonathanhamberg.com/post/cmake-embedding-git-hash/
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Configurations
configure_file (
    "${SRC_DIR}/specgram.hpp.in"
    "${SRC_DIR}/specgram.hpp"
)

# Source setup
set (SPECGRAM_SOURCES
    "${SRC_DIR}/specgram.cpp"
    "${SRC_DIR}/configuration.cpp"
    "${SRC_DIR}/input-parser.cpp"
    "${SRC_DIR}/input-reader.cpp"
    "${SRC_DIR}/color-map.cpp"
    "${SRC_DIR}/value-map.cpp"
    "${SRC_DIR}/window-function.cpp"
    "${SRC_DIR}/fft.cpp"
    "${SRC_DIR}/live.cpp"
    "${SRC_DIR}/renderer.cpp"

    "${SRC_DIR}/share-tech-mono.cpp"
)

# Executable target
add_executable (${PROJECT_NAME} ${SPECGRAM_SOURCES})
target_link_libraries (${PROJECT_NAME} spdlog::spdlog sfml-window sfml-graphics ${FFTW3})

# HTML manpage target
add_custom_target(manpage
    COMMAND groff -mandoc ${MAN_DIR}/specgram.1 -Thtml > ${MAN_DIR}/specgram.1.html
    COMMAND groff -mandoc ${MAN_DIR}/specgram.1 -Tpdf > ${MAN_DIR}/specgram.1.pdf
    DEPENDS ${MAN_DIR}/specgram.1
    BYPRODUCTS ${MAN_DIR}/specgram.1.html ${MAN_DIR}/specgram.1.pdf
)

# Install
install (TARGETS ${PROJECT_NAME} DESTINATION bin)
install (FILES "${MAN_DIR}/specgram.1" DESTINATION "share/man/man1/")
install (FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "share/licenses/specgram/")
install (FILES "${PROJECT_SOURCE_DIR}/share/applications/specgram.desktop"
         DESTINATION "share/applications/")
install (DIRECTORY "${PROJECT_SOURCE_DIR}/share/icons/hicolor"
         DESTINATION "share/icons")