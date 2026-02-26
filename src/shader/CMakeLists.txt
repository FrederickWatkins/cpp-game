add_shader_header(vertex_shader "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert" "${CMAKE_CURRENT_BINARY_DIR}/vertex_source.h" "VERTEX_SOURCE")
add_shader_header(fragment_shader "${CMAKE_CURRENT_SOURCE_DIR}/shader.frag" "${CMAKE_CURRENT_BINARY_DIR}/fragment_source.h" "FRAGMENT_SOURCE")

add_library(shader INTERFACE shader.hpp)
target_link_libraries(shader INTERFACE external vertex_shader fragment_shader)
