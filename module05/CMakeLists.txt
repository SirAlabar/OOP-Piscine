cmake_minimum_required(VERSION 3.14)
project(RailwaySimulation VERSION 1.0 LANGUAGES CXX)

string(ASCII 27 Esc)
set(R "${Esc}[0m")
set(G "${Esc}[32m")
set(Y "${Esc}[33m")
set(C "${Esc}[36m")
set(M "${Esc}[35m")
set(B "${Esc}[34m")

message("${C}=== Railway Simulation ===${R}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Werror -Wpedantic)
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fdiagnostics-color=always>")

include_directories(include)

# ─── Sources ─────────────────────────────────────────────────
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*/rendering/factory/.*")

# ─── Optional SFML ───────────────────────────────────────────
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)

if(SFML_FOUND)
    message("${G}SFML found — rendering enabled${R}")
    list(APPEND ALL_SOURCES "src/rendering/factory/RendererFactory_SFML.cpp")
else()
    message("${Y}SFML not found — headless build${R}")
    list(FILTER ALL_SOURCES EXCLUDE REGEX ".*/rendering/.*")
    list(APPEND ALL_SOURCES "src/rendering/factory/RendererFactory_Stub.cpp")
endif()

# ─── Core library ────────────────────────────────────────────
add_library(RailwaySimCore ${ALL_SOURCES})
message("${B}Core library configured${R}")

if(SFML_FOUND)
    target_link_libraries(RailwaySimCore sfml-graphics sfml-window sfml-system)
endif()

# ─── Main executable ─────────────────────────────────────────
add_executable(Railway_Simulator src/main.cpp)
target_link_libraries(Railway_Simulator RailwaySimCore)
message("${B}Main executable configured${R}")

# ─── Tests ───────────────────────────────────────────────────
option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
    enable_testing()
    find_package(GTest REQUIRED)
    file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
    add_executable(Railway_Simulator_tests ${TEST_SOURCES})
    target_link_libraries(Railway_Simulator_tests RailwaySimCore GTest::GTest GTest::Main)
    add_test(NAME UnitTests COMMAND Railway_Simulator_tests)
    message("${M}Tests enabled${R}")
endif()

message("${C}Build system ready${R}")