#pragma once
#include <algorithm>
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <iterator>
#include <memory>
#include <optional>
#include <vector>

#include "../draw/buffer.h"
#include "../draw/colour.h"
#include "camera.h"

class Triangle
{
  public:
    Triangle() : vertices(), colour(MAGENTA) {};
    Triangle(glm::vec<3, size_t> vertices, Colour colour) : vertices(vertices), colour(colour) {};
    glm::vec<3, size_t> vertices;
    Colour colour;
};

template <typename vertex_t> class Mesh
{
  public:
    /// Create empty mesh
    Mesh() : vertices({}), triangles({}) {};
    /// Create mesh from components
    Mesh(std::vector<vertex_t> vertices, std::vector<Triangle> triangles) : vertices(vertices), triangles(triangles) {};
    /// Add vertex to mesh, returning vertex id
    auto add_vertex(vertex_t &vertex) -> size_t
    {
        auto existing_vertex = std::find(vertices.begin(), vertices.end(), vertex);
        size_t index = std::distance(vertices.begin(), existing_vertex);
        if (existing_vertex == vertices.end())
        {
            vertices.push_back(vertex);
        }
        return index;
    }
    /// Add vertex to mesh from vertex ids, returning triangle id
    auto add_triangle(Triangle triangle) -> size_t
    {
        size_t index = triangles.size();
        triangles.push_back(triangle);
        return index;
    }
    /// Add triangle to mesh from vertex coordinates, returning triangle id
    auto add_triangle(glm::vec<3, vertex_t> &vertices, Colour colour) -> size_t
    {
        auto indices = glm::vec<3, size_t>{0};
        for (size_t i = 0; i < vertices.length(); i++)
        {
            indices[i] = add_vertex(vertices[i]);
        }
        return add_triangle(Triangle(indices, colour));
    }
    inline auto num_vertices() -> size_t
    {
        return vertices.size();
    }
    inline auto num_triangles() -> size_t
    {
        return triangles.size();
    }
    /// Get vertex with id id
    inline auto get_vertex(size_t id) -> vertex_t
    {
        return vertices[id];
    }
    /// Get triangle with id id
    inline auto get_triangle(size_t id) -> Triangle
    {
        return triangles[id];
    }

  private:
    std::vector<vertex_t> vertices;
    std::vector<Triangle> triangles;
};

class NdcMesh : public Mesh<glm::vec3>
{
    using Mesh::Mesh;

  public:
    /// Rasterize to pixel buffer
    void write_to_buffer(Buffer &buffer);

  private:
    /// Determine pixel colour
    auto pixel_colour(float x_ndc, float y_ndc) -> std::optional<Colour>;
};

class ClipSpaceMesh : public Mesh<glm::vec4>
{
    using Mesh::Mesh;

  public:
    /// Convert mesh from clip space to normalized device coords
    auto to_ndc() -> NdcMesh;
};

class WorldSpaceMesh : public Mesh<glm::vec3>
{
    using Mesh::Mesh;

  public:
    auto to_clip(Camera &camera) -> ClipSpaceMesh;
};
