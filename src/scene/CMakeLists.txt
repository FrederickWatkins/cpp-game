add_library(scene mesh.hpp node.hpp scene.hpp camera.hpp light.hpp scene.cpp camera.cpp)

target_link_libraries(scene PUBLIC external shader)
