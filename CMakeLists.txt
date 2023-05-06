cmake_minimum_required (VERSION 3.10)

project (specgram LANGUAGES CXX)

# Version
set (VERSION_MAJOR 0)
set (VERSION_MINOR 9)
set (VERSION_PATCH 3)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option (TESTING "Build test targets" OFF)

# Dependencies
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)
find_package (SFML 2.5 COMPONENTS window graphics REQUIRED)
find_library (FFTW3 fftw3)

if (TESTING)
    find_package(GTest)
    find_package(X11)
endif()

# Compiler setup
add_compile_options (-Wall -Wextra -pedantic)

# Directory setup
set (SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set (MAN_DIR "${PROJECT_SOURCE_DIR}/man")

include_directories (${SOURCE_DIR} ${X11_INCLUDE_DIR})

# Get the latest abbreviated commit hash of the working branch
# https://jonathanhamberg.com/post/cmake-embedding-git-hash/
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if (NOT ${GIT_HASH} STREQUAL "")
    add_definitions(-DGIT_BUILD)
endif()

# Configurations
configure_file (
    "${SRC_DIR}/specgram.hpp.in"
    "${SRC_DIR}/specgram.hpp"
)

# Source setup
set (SPECGRAM_SOURCES
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

# Static lib (we build this once for the executable and the unit tests)
add_library (${PROJECT_NAME}_static STATIC ${SPECGRAM_SOURCES})

# Executable target
add_executable (${PROJECT_NAME} ${SRC_DIR}/specgram.cpp)
target_link_libraries (${PROJECT_NAME} ${PROJECT_NAME}_static Threads::Threads sfml-window sfml-graphics ${FFTW3})

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

if (TESTING)
    set (UNIT_TEST_SOURCES
        test/test.cpp
        test/test-fft.cpp
        test/test-renderer.cpp
        test/test-input-reader.cpp
        test/test-input-parser.cpp
        test/test-color-map.cpp
        test/test-value-map.cpp
        test/test-window-function.cpp
    )

    # Unit tests
    enable_testing ()
    add_executable(unittest ${UNIT_TEST_SOURCES})
    target_link_libraries (unittest GTest::GTest ${PROJECT_NAME}_static Threads::Threads sfml-graphics ${FFTW3} ${X11_LIBRARIES})
    gtest_discover_tests (unittest)
endif()
