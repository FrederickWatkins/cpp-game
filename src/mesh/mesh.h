#include "../shader/shader.h"
#include "../vertex/vao.h"
#include <assimp/mesh.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <vector>

template <bool has_colour, bool has_normal, size_t num_tex_coords, typename... uniforms> class Mesh
{
    using Vertex = VertexAttributes<has_colour, has_normal, num_tex_coords>;

  public:
    Mesh(aiMesh &mesh, ShaderProgram &shader_program) : shader_program(shader_program)
    {
        add_mesh(mesh);
    }
    Mesh(std::vector<Vertex> &vertices, ShaderProgram &shader_program) : shader_program(shader_program)
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
                throw std::runtime_error("Mesh missing required colour data");
        }
        if constexpr (num_tex_coords > 0)
        {
            if (!mesh.HasTextureCoords(0))
                throw std::runtime_error("Mesh missing required texture data");
        }

        for (unsigned int f = 0; f < mesh.mNumFaces; f++)
        {
            aiFace &face = mesh.mFaces[f];
            for (unsigned int i = 0; i < face.mNumIndices; i++)
            {
                unsigned int idx = face.mIndices[i];
                Vertex v;

                v.position = {mesh.mVertices[idx].x, mesh.mVertices[idx].y, mesh.mVertices[idx].z};

                if constexpr (has_normal)
                {
                    v.normal = {mesh.mNormals[idx].x, mesh.mNormals[idx].y, mesh.mNormals[idx].z};
                }

                if constexpr (has_colour)
                {
                    aiColor4D c = mesh.mColors[0][idx];
                    v.colour = {c.r, c.g, c.b, c.a};
                }

                if constexpr (num_tex_coords >= 1)
                    v.tex_coords[0] = mesh.mTextureCoords[0][idx][0];
                if constexpr (num_tex_coords >= 2)
                    v.tex_coords[1] = mesh.mTextureCoords[0][idx][1];
                if constexpr (num_tex_coords >= 3)
                    v.tex_coords[2] = mesh.mTextureCoords[0][idx][2];

                vertices.push_back(v);
            }
        }
        add_mesh(vertices);
    }
    void add_mesh(std::vector<Vertex> &vertices)
    {
        vao.use();
        vao.add_vbo(vertices);
    }
    void draw(std::array<GLint, sizeof...(uniforms)> &uniform_locations, uniforms &...uniform_values)
    {
        shader_program.use();
        vao.use();
        set_uniforms(uniform_locations, std::index_sequence_for<uniforms...>{}, uniform_values...);
        vao.draw();
    }

  private:
    template <std::size_t... I>
    void set_uniforms(
        const std::array<GLint, sizeof...(uniforms)> &locations, std::index_sequence<I...>, const uniforms &...values
    )
    {
        // Fold expression: calls set_uniform for each location/value pair
        (set_uniform(locations[I], values), ...);
    }
    // --- OVERLOADS FOR ALL SUPPORTED OPENGL TYPES ---
    // clang-format off
    // Scalars
    void set_uniform(GLint location, float value) const { glUniform1f(location, value); }
    void set_uniform(GLint location, int value) const { glUniform1i(location, value); }
    void set_uniform(GLint location, unsigned int value) const { glUniform1ui(location, value); }
    void set_uniform(GLint location, bool value) const { glUniform1i(location, value); } // OpenGL uses 1i for bools

    // GLM Vectors
    void set_uniform(GLint location, const glm::vec2& value) const { glUniform2fv(location, 1, glm::value_ptr(value)); }
    void set_uniform(GLint location, const glm::vec3& value) const { glUniform3fv(location, 1, glm::value_ptr(value)); }
    void set_uniform(GLint location, const glm::vec4& value) const { glUniform4fv(location, 1, glm::value_ptr(value)); }
    
    void set_uniform(GLint location, const glm::ivec2& value) const { glUniform2iv(location, 1, glm::value_ptr(value)); }
    void set_uniform(GLint location, const glm::ivec3& value) const { glUniform3iv(location, 1, glm::value_ptr(value)); }
    void set_uniform(GLint location, const glm::ivec4& value) const { glUniform4iv(location, 1, glm::value_ptr(value)); }

    void set_uniform(GLint location, const glm::uvec2& value) const { glUniform2uiv(location, 1, glm::value_ptr(value)); }
    void set_uniform(GLint location, const glm::uvec3& value) const { glUniform3uiv(location, 1, glm::value_ptr(value)); }
    void set_uniform(GLint location, const glm::uvec4& value) const { glUniform4uiv(location, 1, glm::value_ptr(value)); }

    // GLM Matrices
    void set_uniform(GLint location, const glm::mat2& value) const { glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
    void set_uniform(GLint location, const glm::mat3& value) const { glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
    void set_uniform(GLint location, const glm::mat4& value) const { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
    // clang-format on
    // --- OVERLOADS FOR ARRAYS / CONTAINERS ---

    // std::vector of GLM Matrices (Dynamic arrays, perfect for bone transforms)
    void set_uniform(GLint location, const std::vector<glm::mat4> &values) const
    {
        if (!values.empty())
        {
            // Note the 'count' parameter is now values.size(), not 1
            glUniformMatrix4fv(location, static_cast<GLsizei>(values.size()), GL_FALSE, glm::value_ptr(values[0]));
        }
    }

    // std::array of GLM Matrices (Compile-time sized arrays)
    template <size_t N> void set_uniform(GLint location, const std::array<glm::mat4, N> &values) const
    {
        if constexpr (N > 0)
        {
            glUniformMatrix4fv(location, N, GL_FALSE, glm::value_ptr(values[0]));
        }
    }

    // std::vector of floats (e.g., for morph target weights)
    void set_uniform(GLint location, const std::vector<float> &values) const
    {
        if (!values.empty())
        {
            glUniform1fv(location, static_cast<GLsizei>(values.size()), values.data());
        }
    }
    ShaderProgram &shader_program;
    VertexArrayObject<has_colour, has_normal, num_tex_coords> vao;
};
