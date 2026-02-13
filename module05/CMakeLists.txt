cmake_minimum_required(VERSION 3.14)
project(RailwaySimulation VERSION 1.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
add_compile_options(-Wall -Wextra -Werror -Wpedantic)

# Include directories
include_directories(include)

# Collect source files (excluding main.cpp to avoid multiple main() definitions)
file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

# SFML (optional in host, expected in Docker image)
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)
if(SFML_FOUND)
    add_compile_definitions(HAVE_SFML)
else()
    message(WARNING "SFML not found: --render mode will be unavailable on this host build")
    list(FILTER LIB_SOURCES EXCLUDE REGEX ".*src/rendering/.*\\.cpp$")
endif()

# Create library with core logic (shared between main executable and tests)
add_library(RailwaySimCore ${LIB_SOURCES})

# Main executable
add_executable(Railway_Simulator src/main.cpp)
target_link_libraries(Railway_Simulator RailwaySimCore)
if(SFML_FOUND)
    target_link_libraries(Railway_Simulator sfml-graphics sfml-window sfml-system)
endif()

# Testing
enable_testing()

# GoogleTest
find_package(GTest REQUIRED)

# Collect test files
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

# Test executable
add_executable(Railway_Simulator_tests ${TEST_SOURCES})
target_link_libraries(Railway_Simulator_tests RailwaySimCore GTest::GTest GTest::Main)
if(SFML_FOUND)
    target_link_libraries(Railway_Simulator_tests sfml-graphics sfml-window sfml-system)
endif()

# Register tests with ctest
add_test(NAME UnitTests COMMAND Railway_Simulator_tests)
