add_subdirectory(shader)
add_subdirectory(vertex)

add_library(src INTERFACE)

target_link_libraries(src INTERFACE shader vertex)

