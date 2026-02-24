add_shader_header(vertex_shader "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert" "${CMAKE_CURRENT_BINARY_DIR}/vertex_shader.h" "VERTEX_SHADER_SOURCE")
add_shader_header(fragment_shader "${CMAKE_CURRENT_SOURCE_DIR}/shader.frag" "${CMAKE_CURRENT_BINARY_DIR}/fragment_shader.h" "FRAGMENT_SHADER_SOURCE")
add_library(shader INTERFACE shader.h)
target_link_libraries(shader INTERFACE vertex_shader fragment_shader)
