cmake_minimum_required(VERSION 3.20)
project(Game LANGUAGES CXX)
add_executable(game src/main.cpp)

list(APPEND CMAKE_PROJECT_TOP_LEVEL_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/conan_provider.cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Ensures configuration fails if compiler lacks C++17 support
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(game PRIVATE /W4 /WX) # Level 4 warnings, treat as errors
else()
    target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

find_package(SDL3 REQUIRED)
target_link_libraries(game PRIVATE SDL3::SDL3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)