add_subdirectory(shader)
add_subdirectory(scene)

add_library(src INTERFACE)

target_link_libraries(src INTERFACE shader scene)

