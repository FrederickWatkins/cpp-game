add_subdirectory(draw)

add_executable(game main.cpp)

if(MSVC)
    target_compile_options(game PRIVATE /W4 /WX) # Level 4 warnings, treat as errors
else()
    target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(SDL3 REQUIRED)
target_link_libraries(game PRIVATE SDL3::SDL3 draw)
