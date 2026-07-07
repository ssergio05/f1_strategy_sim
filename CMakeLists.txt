cmake_minimum_required(VERSION 3.14)

project(f1_strategy_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.12.1
)
FetchContent_MakeAvailable(googletest)

add_library(f1sim_core STATIC
    src/Car.cpp
    src/CSVExporter.cpp
    src/MonteCarloEngine.cpp
    src/RaceSession.cpp
    src/TyreModel.cpp
)

target_include_directories(f1sim_core PUBLIC include)

add_executable(f1_simulator src/main.cpp)
target_link_libraries(f1_simulator PRIVATE f1sim_core)

enable_testing()
add_subdirectory(tests)
