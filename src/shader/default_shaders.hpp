#pragma once
#include <glm/glm.hpp>

#include "shader.hpp"
#include "../vertex/vao.hpp"
#include "../mesh/mesh.hpp"

/// Shader from NDC space, with vertex colours, no texture mapping, no lighting
auto shader_ncnn() -> ShaderProgram;
using MeshNCNN = Mesh<true, false, 0>;
using VertexAttributesNCNN = VertexAttributes<true, false, 0>;

/// Shader from world space, with vertex colours, no texture mapping, no lighting
auto shader_wcnn() -> ShaderProgram;
auto uniform_locations_wcnn() -> std::array<GLuint, 1>;
using MeshWCNN = Mesh<true, false, 0, glm::mat4>;
using VertexAttributesWCNN = VertexAttributes<true, false, 0>;


/// Shader from world space, with vertex colours, no texture mapping, phong lighting
auto shader_wcnp() -> ShaderProgram;
auto uniform_locations_wcnp() -> std::array<GLuint, 5>;
using MeshWCNP = Mesh<true, true, 0, glm::mat4, glm::vec3, glm::vec3, glm::vec3, Material>;
using VertexAttributesWCNP = VertexAttributes<true, true, 0>;
