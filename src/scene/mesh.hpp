#pragma once

#include <assimp/mesh.h>
#include <cstddef>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stdexcept>
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

class VirtualMesh
{
  public:
    VirtualMesh() = default;
    VirtualMesh(const VirtualMesh &) = default;
    VirtualMesh(VirtualMesh &&) = delete;
    auto operator=(const VirtualMesh &) -> VirtualMesh & = default;
    auto operator=(VirtualMesh &&) -> VirtualMesh & = delete;
    virtual ~VirtualMesh() = default;
    virtual void use() = 0;
    virtual void draw() = 0;
};

template <bool has_colour, bool has_normal, size_t num_tex_coords> class Mesh : public VirtualMesh
{
    using Vertex = VertexAttributes<has_colour, has_normal, num_tex_coords>;

  public:
    explicit Mesh(std::vector<Vertex> &vertices)
    {
        add_vertices(vertices);
    }
    explicit Mesh(aiMesh &mesh)
    {
        std::vector<Vertex> vertices;
        // Validation for required attributes
        if constexpr (has_colour)
        {
            if (!mesh.HasVertexColors(0))
            {
                throw std::runtime_error("Mesh missing required colour data");
            }
        }
        if constexpr (num_tex_coords > 0)
        {
            if (!mesh.HasTextureCoords(0))
            {
                throw std::runtime_error("Mesh missing required texture data");
            }
        }
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (unsigned int face_idx = 0; face_idx < mesh.mNumFaces; face_idx++)
        {
            aiFace &face = mesh.mFaces[face_idx];
            for (unsigned int i = 0; i < face.mNumIndices; i++)
            {
                unsigned int idx = face.mIndices[i];
                Vertex vertex;

                vertex.position = {mesh.mVertices[idx].x, mesh.mVertices[idx].y, mesh.mVertices[idx].z};

                if constexpr (has_normal)
                {
                    vertex.normal = {mesh.mNormals[idx].x, mesh.mNormals[idx].y, mesh.mNormals[idx].z};
                }

                if constexpr (has_colour)
                {
                    aiColor4D colour = mesh.mColors[0][idx];
                    vertex.colour = {colour.r, colour.g, colour.b, colour.a};
                }

                if constexpr (num_tex_coords >= 1)
                {
                    vertex.tex_coords[0] = mesh.mTextureCoords[0][idx][0];
                }
                if constexpr (num_tex_coords >= 2)
                {
                    vertex.tex_coords[1] = mesh.mTextureCoords[0][idx][1];
                }
                if constexpr (num_tex_coords >= 3)
                {
                    vertex.tex_coords[2] = mesh.mTextureCoords[0][idx][2];
                }

                vertices.push_back(vertex);
            }
        }
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        add_vertices(vertices);
    }
    void use() override
    {
        glBindVertexArray(VAO);
    }
    void draw() override
    {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }

  private:
    void add_vertices(std::vector<Vertex> vertices)
    {
        glGenVertexArrays(1, &VAO);
        use();
        unsigned int VBO = 0;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        // Position
        add_attribute_pointer(0, 3, GL_FLOAT, offsetof(Vertex, position));
        // Colour
        if constexpr (has_colour)
        {
            add_attribute_pointer(1, 4, GL_FLOAT, offsetof(Vertex, colour));
        }
        // Texture coordinates
        static_assert(num_tex_coords <= 3, "Texture coordinates over 3d are not supported");
        if constexpr (num_tex_coords > 0 && num_tex_coords <= 3)
        {
            add_attribute_pointer(2, num_tex_coords, GL_FLOAT, offsetof(Vertex, tex_coords));
        }
        // Normals
        if constexpr (has_normal)
        {
            add_attribute_pointer(3, 3, GL_FLOAT, offsetof(Vertex, normal));
        }
        count = vertices.size();
        // Unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
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
};