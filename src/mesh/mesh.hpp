#pragma once
#include "../shader/shader.hpp"
#include "../vertex/vao.hpp"
#include <assimp/mesh.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

template <bool NDC, bool has_colour, bool has_lighting, size_t num_tex_coords> class Mesh
{
    using Vertex = VertexAttributes<has_colour, has_lighting, num_tex_coords>;

  public:
    Mesh(aiMesh &mesh, std::shared_ptr<ShaderProgram<NDC, has_colour, has_lighting, num_tex_coords>> shader_program)
        : shader_program(std::move(shader_program))
    {
        add_mesh(mesh);
    }
    Mesh(
        std::vector<Vertex> &vertices,
        std::shared_ptr<ShaderProgram<NDC, has_colour, has_lighting, num_tex_coords>> shader_program
    )
        : shader_program(std::move(shader_program))
    {
        add_mesh(vertices);
    }
    void add_mesh(aiMesh &mesh)
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

                if constexpr (has_lighting)
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
        add_mesh(vertices);
    }
    void add_mesh(std::vector<Vertex> &vertices)
    {
        vao.use();
        vao.add_vbo(vertices);
    }
    void draw()
    {
        shader_program->use();
        vao.use();
        vao.draw();
    }

  private:
    std::shared_ptr<ShaderProgram<NDC, has_colour, has_lighting, num_tex_coords>> shader_program;
    VertexArrayObject<has_colour, has_lighting, num_tex_coords> vao;
};
