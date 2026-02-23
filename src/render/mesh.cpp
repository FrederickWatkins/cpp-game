#include "mesh.h"

#include <cstddef>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

// NDC mesh

auto inside_triangle(float x_ndc, float y_ndc, glm::vec<3, glm::vec3> &vertices) -> bool
{
    auto ab = vertices[1] - vertices[0];
    auto bc = vertices[2] - vertices[1];
    auto ca = vertices[0] - vertices[2];

    auto p = glm::vec3(x_ndc, y_ndc, 0);
    auto ap = p - vertices[0];
    auto bp = p - vertices[1];
    auto cp = p - vertices[2];

    auto d1 = glm::cross(ab, ap).z;
    auto d2 = glm::cross(bc, bp).z;
    auto d3 = glm::cross(ca, cp).z;

    bool all_neg = (d1 <= 0) && (d2 <= 0) && (d3 <= 0);
    bool all_pos = (d1 >= 0) && (d2 >= 0) && (d3 >= 0);
    return all_neg || all_pos;
}

auto NdcMesh::pixel_colour(float x_ndc, float y_ndc) -> std::optional<Colour>
{
    float z_buffer = FLT_MAX;
    std::optional<Colour> colour = std::nullopt;
    for (size_t i = 0; i < num_triangles(); i++)
    {
        auto triangle = get_triangle(i);
        glm::vec<3, glm::vec3> vertices;
        for (size_t j = 0; j < triangle.vertices.length(); j++)
        {
            vertices[j] = get_vertex(triangle.vertices[j]);
        }
        float depth = vertices[0].z + vertices[1].z + vertices[2].z;
        if (inside_triangle(x_ndc, y_ndc, vertices) && depth < z_buffer)
        {
            z_buffer = depth;
            colour = triangle.colour;
        }
    }
    return colour;
}

void NdcMesh::write_to_buffer(Buffer &buffer)
{
    for (size_t x = 0; x < buffer.width; x++)
    {
        for (size_t y = 0; y < buffer.height; y++)
        {
            auto x_ndc = (2.0f * static_cast<float>(x) / static_cast<float>(buffer.width)) - 1.0f;
            auto y_ndc = 1.0f - 2.0f * (static_cast<float>(y) / static_cast<float>(buffer.height));
            auto colour = pixel_colour(x_ndc, y_ndc);
            if (colour.has_value())
            {
                buffer.write_pixel(x, y, *colour);
            }
        }
    }
}

auto ClipSpaceMesh::to_ndc() -> NdcMesh
{
    std::vector<glm::vec3> ndc_vertices(num_vertices());
    std::vector<Triangle> ndc_triangles(num_triangles());
    for (size_t i = 0; i < num_vertices(); i++)
    {
        auto clip_vertex = get_vertex(i);
        auto ndc_vertex = glm::vec3(clip_vertex) / clip_vertex.w;
        ndc_vertices[i] = ndc_vertex;
    }
    for (size_t i = 0; i < num_triangles(); i++)
    {
        ndc_triangles[i] = get_triangle(i);
    }
    return NdcMesh(ndc_vertices, ndc_triangles);
}

// World space mesh

auto WorldSpaceMesh::to_clip(Camera &camera) -> ClipSpaceMesh
{
    std::vector<glm::vec4> clip_vertices(num_vertices());
    std::vector<Triangle> clip_triangles(num_triangles());
    glm::mat4 view_mat = glm::lookAt(camera.pos, camera.pos + camera.view_normal, camera.up_vector);
    glm::mat4 proj_mat =
        glm::perspective(glm::radians(camera.fov_degrees), camera.aspect_ratio, camera.near_clip, camera.far_clip);
    glm::mat4 transform_mat = proj_mat * view_mat;
    for (size_t i = 0; i < num_vertices(); i++)
    {
        auto world_vertex = glm::vec4(get_vertex(i), 1.0f);
        auto clip_vertex = transform_mat * world_vertex;
        clip_vertices[i] = clip_vertex;
    }
    for (size_t i = 0; i < num_triangles(); i++)
    {
        clip_triangles[i] = get_triangle(i);
    }
    return ClipSpaceMesh(clip_vertices, clip_triangles);
}