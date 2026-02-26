add_shader_header(vertex_wcnn "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert" "${CMAKE_CURRENT_BINARY_DIR}/vertex_wcnn.h" "VERTEX_WCNN" "${CMAKE_CURRENT_SOURCE_DIR}/defines/wcnn.h")
add_shader_header(fragment_wcnn "${CMAKE_CURRENT_SOURCE_DIR}/shader.frag" "${CMAKE_CURRENT_BINARY_DIR}/fragment_wcnn.h" "FRAGMENT_WCNN" "${CMAKE_CURRENT_SOURCE_DIR}/defines/wcnn.h")
add_shader_header(vertex_ncnn "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert" "${CMAKE_CURRENT_BINARY_DIR}/vertex_ncnn.h" "VERTEX_NCNN" "${CMAKE_CURRENT_SOURCE_DIR}/defines/ncnn.h")
add_shader_header(vertex_wcnp "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert" "${CMAKE_CURRENT_BINARY_DIR}/vertex_wcnp.h" "VERTEX_WCNP" "${CMAKE_CURRENT_SOURCE_DIR}/defines/wcnp.h")
add_shader_header(fragment_wcnp "${CMAKE_CURRENT_SOURCE_DIR}/shader.frag" "${CMAKE_CURRENT_BINARY_DIR}/fragment_wcnp.h" "FRAGMENT_WCNP" "${CMAKE_CURRENT_SOURCE_DIR}/defines/wcnp.h")

add_library(shader shader.hpp shader.cpp)
target_link_libraries(shader PUBLIC external vertex_wcnn fragment_wcnn vertex_ncnn vertex_wcnp fragment_wcnp)
