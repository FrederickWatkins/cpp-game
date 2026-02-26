#pragma once
#include "fragment_source.h"
#include "vertex_source.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename T> class Uniform
{
  public:
    Uniform() : location(0) {};
    explicit Uniform(GLuint location) : location(location) {};
    void set(T value)
    {
        set_uniform(value);
    }

  private:
    GLuint location;
    // --- OVERLOADS FOR ALL SUPPORTED OPENGL TYPES ---
    // clang-format off
    // Scalars
    void set_uniform(float value) const { glUniform1f(location, value); }
    void set_uniform(int value) const { glUniform1i(location, value); }
    void set_uniform(unsigned int value) const { glUniform1ui(location, value); }
    void set_uniform(bool value) const { glUniform1i(location, static_cast<GLint>(value)); } // OpenGL uses 1i for bools

    // GLM Vectors
    void set_uniform(const glm::vec2& value) const { glUniform2fv(location, 1, glm::value_ptr(value)); }
    void set_uniform(const glm::vec3& value) const { glUniform3fv(location, 1, glm::value_ptr(value)); }
    void set_uniform(const glm::vec4& value) const { glUniform4fv(location, 1, glm::value_ptr(value)); }
    
    void set_uniform(const glm::ivec2& value) const { glUniform2iv(location, 1, glm::value_ptr(value)); }
    void set_uniform(const glm::ivec3& value) const { glUniform3iv(location, 1, glm::value_ptr(value)); }
    void set_uniform(const glm::ivec4& value) const { glUniform4iv(location, 1, glm::value_ptr(value)); }

    void set_uniform(const glm::uvec2& value) const { glUniform2uiv(location, 1, glm::value_ptr(value)); }
    void set_uniform(const glm::uvec3& value) const { glUniform3uiv(location, 1, glm::value_ptr(value)); }
    void set_uniform(const glm::uvec4& value) const { glUniform4uiv(location, 1, glm::value_ptr(value)); }

    // GLM Matrices
    void set_uniform(const glm::mat2& value) const { glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
    void set_uniform(const glm::mat3& value) const { glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
    void set_uniform(const glm::mat4& value) const { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)); }

    // clang-format on
    // --- OVERLOADS FOR ARRAYS / CONTAINERS ---

    // std::vector of GLM Matrices (Dynamic arrays, perfect for bone transforms)
    void set_uniform(const std::vector<glm::mat4> &values) const
    {
        if (!values.empty())
        {
            // Note the 'count' parameter is now values.size(), not 1
            glUniformMatrix4fv(location, static_cast<GLsizei>(values.size()), GL_FALSE, glm::value_ptr(values[0]));
        }
    }

    // std::array of GLM Matrices (Compile-time sized arrays)
    template <size_t N> void set_uniform(const std::array<glm::mat4, N> &values) const
    {
        if constexpr (N > 0)
        {
            glUniformMatrix4fv(location, N, GL_FALSE, glm::value_ptr(values[0]));
        }
    }

    // std::vector of floats (e.g., for morph target weights)
    void set_uniform(const std::vector<float> &values) const
    {
        if (!values.empty())
        {
            glUniform1fv(location, static_cast<GLsizei>(values.size()), values.data());
        }
    }
};

template <typename T> class UniformBufferObject
{
};

// clang-format off
struct empty_transform_mat{};
struct empty_material{};
struct empty_light_pos{};
struct empty_light_colour{};
struct empty_intensities{};
struct empty_view_pos{};
// clang-format on

struct MaterialUniforms
{
    Uniform<glm::vec3> ambient;
    Uniform<glm::vec3> diffuse;
    Uniform<glm::vec3> specular;
    Uniform<float> shininess;
};

template <bool NDC, bool has_lighting> struct ShaderUniforms
{
    Uniform<glm::mat4> transform_mat;
    [[no_unique_address]] std::conditional_t<!NDC, Uniform<glm::mat4>, empty_transform_mat> projection_mat;
    [[no_unique_address]] std::conditional_t<has_lighting, MaterialUniforms, empty_material> material;
    [[no_unique_address]] std::conditional_t<has_lighting, Uniform<glm::vec3>, empty_light_pos> light_pos;
    [[no_unique_address]] std::conditional_t<has_lighting, Uniform<glm::vec3>, empty_light_colour> light_colour;
    [[no_unique_address]] std::conditional_t<has_lighting, Uniform<glm::vec3>, empty_intensities> intensities;
    [[no_unique_address]] std::conditional_t<has_lighting, Uniform<glm::vec3>, empty_view_pos> view_pos;
};

template <bool NDC, bool has_colour, bool has_lighting, size_t num_tex_coords> class ShaderProgram
{
  public:
    ShaderProgram()
    {
        const GLint info_log_size = 512;
        std::string vertex_source = VERTEX_SOURCE;
        std::string fragment_source = FRAGMENT_SOURCE;
        std::string replace_string = "#define DEFINES";
        std::string replace_with;

        if constexpr (NDC)
        {
            replace_with.append("#define NDC\n");
        }
        if constexpr (has_colour)
        {
            replace_with.append("#define VERTEX_COLOUR\n");
        }
        if constexpr (has_lighting)
        {
            replace_with.append("#define LIGHTING\n");
        }
        if constexpr (num_tex_coords == 1)
        {
            replace_with.append("#define TEXTURE_COORDS_1D\n");
        }
        if constexpr (num_tex_coords == 2)
        {
            replace_with.append("#define TEXTURE_COORDS_2D\n");
        }
        if constexpr (num_tex_coords == 3)
        {
            replace_with.append("#define TEXTURE_COORDS_3D\n");
        }

        size_t pos = 0;
        pos = vertex_source.find(replace_string);
        vertex_source.replace(pos, replace_string.length(), replace_with);
        pos = fragment_source.find(replace_string);
        fragment_source.replace(pos, replace_string.length(), replace_with);

        unsigned int vertex_shader = 0;
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        auto *vertex_src_ptr = vertex_source.begin().base();
        glShaderSource(vertex_shader, 1, &vertex_src_ptr, nullptr);
        glCompileShader(vertex_shader);

        int vertex_success = 0;
        std::string info_log;
        info_log.resize(info_log_size);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_success);
        if (vertex_success == 0)
        {
            glGetShaderInfoLog(vertex_shader, info_log_size, nullptr, info_log.begin().base());
            throw std::runtime_error(std::string("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n").append(info_log));
        }

        unsigned int fragment_shader = 0;
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        auto *fragment_src_ptr = fragment_source.begin().base();
        glShaderSource(fragment_shader, 1, &fragment_src_ptr, nullptr);
        glCompileShader(fragment_shader);

        int fragment_success = 0;
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_success);
        if (fragment_success == 0)
        {
            glGetShaderInfoLog(fragment_shader, info_log_size, nullptr, info_log.data());
            throw std::runtime_error(std::string("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n").append(info_log));
        }


        program = glCreateProgram();

        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        int link_success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &link_success);
        if (link_success == 0)
        {
            glGetProgramInfoLog(program, info_log_size, nullptr, info_log.begin().base());
            throw std::runtime_error(std::string("ERROR::SHADER::LINK_FAILED\n").append(info_log));
        }
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }
    void use() const
    {
        glUseProgram(program);
    }
    [[nodiscard]] auto get_uniforms() const -> ShaderUniforms<NDC, has_lighting>
    {
        use();
        ShaderUniforms<NDC, has_lighting> uniforms;
        uniforms.transform_mat = Uniform<glm::mat4>(glGetUniformLocation(program, "transform_mat"));
        if constexpr (!NDC)
        {
            uniforms.projection_mat = Uniform<glm::mat4>(glGetUniformLocation(program, "projection_mat"));
        }
        if constexpr (has_lighting)
        {
            MaterialUniforms material_uniforms;
            material_uniforms.ambient = Uniform<glm::vec3>(glGetUniformLocation(program, "material.ambient"));
            material_uniforms.diffuse = Uniform<glm::vec3>(glGetUniformLocation(program, "material.diffuse"));
            material_uniforms.specular = Uniform<glm::vec3>(glGetUniformLocation(program, "material.specular"));
            material_uniforms.shininess = Uniform<float>(glGetUniformLocation(program, "material.shininess"));
            uniforms.material = material_uniforms;
            uniforms.light_pos = Uniform<glm::vec3>(glGetUniformLocation(program, "light_pos"));
            uniforms.light_colour = Uniform<glm::vec3>(glGetUniformLocation(program, "light_colour"));
            uniforms.intensities = Uniform<glm::vec3>(glGetUniformLocation(program, "intensities"));
            uniforms.view_pos = Uniform<glm::vec3>(glGetUniformLocation(program, "view_pos"));
        }
        return uniforms;
    }

  private:
    unsigned int program;
};
