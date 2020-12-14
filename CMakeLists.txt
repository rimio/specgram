cmake_minimum_required (VERSION 3.1)

project (specgram LANGUAGES CXX)

# Version
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
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
include_directories (${SOURCE_DIR})

# Configurations
configure_file (
    "${SRC_DIR}/specgram.hpp.in"
    "${SRC_DIR}/specgram.hpp"
)

# Source setup
set (SPECGRAM_SOURCES
    "${SRC_DIR}/specgram.cpp"
    "${SRC_DIR}/configuration.cpp"
    "${SRC_DIR}/input.cpp"
    "${SRC_DIR}/fft.cpp"
    "${SRC_DIR}/colormap.cpp"
)

# Targets
add_executable (${PROJECT_NAME} ${SPECGRAM_SOURCES})
target_link_libraries (${PROJECT_NAME} spdlog::spdlog sfml-window sfml-graphics ${FFTW3})
install (TARGETS ${PROJECT_NAME} DESTINATION bin)