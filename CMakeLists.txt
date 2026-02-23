cmake_minimum_required(VERSION 3.20)
project(Game LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Ensures configuration fails if compiler lacks C++17 support
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



add_subdirectory(external)


add_executable(game src/main.cpp)

if(MSVC)
    target_compile_options(game PRIVATE /W4 /WX) # Level 4 warnings, treat as errors
else()
    target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(SDL3 REQUIRED)
find_package(glm CONFIG REQUIRED)
target_include_directories(game PRIVATE external/include/)
target_link_libraries(game PRIVATE SDL3::SDL3 PRIVATE glm::glm external)

