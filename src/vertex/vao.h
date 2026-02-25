#pragma once

#include <cstddef>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct empty_colour
{
};

struct empty_normal
{
};

struct empty_tex
{
};

template <bool has_colour, bool has_normal, size_t num_tex_coords> struct VertexAttributes
{
    glm::vec3 position;

    // Optional members
    [[no_unique_address]] std::conditional_t<has_colour, glm::vec4, empty_colour> colour;
    [[no_unique_address]] std::conditional_t<has_normal, glm::vec3, empty_normal> normal;
    [[no_unique_address]] std::conditional_t<(num_tex_coords > 0), glm::vec<num_tex_coords, float>, empty_tex>
        tex_coords;
};

template <bool has_colour, bool has_normal, size_t num_tex_coords> class VertexArrayObject
{
  public:
    VertexArrayObject()
    {
        glGenVertexArrays(1, &VAO);
    }
    void use()
    {
        glBindVertexArray(VAO);
    }
    void add_vbo(std::vector<VertexAttributes<has_colour, has_normal, num_tex_coords>> &vertex_data)
    {
        use();
        unsigned int VBO = 0;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(Vertex), vertex_data.data(), GL_STATIC_DRAW);
        // Position
        add_attribute_pointer(0, 3, GL_FLOAT, offsetof(Vertex, position));
        // Colour
        if constexpr (has_colour)
        {
            add_attribute_pointer(1, 4, GL_FLOAT, offsetof(Vertex, colour));
        }
        // Normals
        if constexpr (has_normal)
        {
            add_attribute_pointer(2, 3, GL_FLOAT, offsetof(Vertex, normal));
        }
        // Texture coordinates
        static_assert(num_tex_coords <= 3, "Texture coordinates over 3d are not supported");
        if constexpr (num_tex_coords > 0 && num_tex_coords <= 3)
        {
            add_attribute_pointer(3, num_tex_coords, GL_FLOAT, offsetof(Vertex, tex_coords));
        }
        count = vertex_data.size();
        // Unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    void draw()
    {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }

  private:
    void add_attribute_pointer(size_t location, GLint size, size_t type, size_t offset)
    {
        glVertexAttribPointer(
            location,
            size,
            type,
            GL_FALSE,
            sizeof(VertexAttributes<has_colour, has_normal, num_tex_coords>),
            reinterpret_cast<void *>(offset) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        );
        glEnableVertexAttribArray(location);
    }
    unsigned int VAO{};
    unsigned int count{};
    using Vertex = VertexAttributes<has_colour, has_normal, num_tex_coords>;
};