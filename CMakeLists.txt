cmake_minimum_required(VERSION 3.20)
project(Game LANGUAGES CXX)
add_subdirectory(src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Ensures configuration fails if compiler lacks C++17 support
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
