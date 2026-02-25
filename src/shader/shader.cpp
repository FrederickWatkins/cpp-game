#include "shader.hpp"
#include <stdexcept>
#include <string>

#include "fragment_wcnn.h"
#include "vertex_ncnn.h"
#include "vertex_wcnn.h"

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
ShaderProgram::ShaderProgram(const char *vertex_source, const char *fragment_source)
{
    const GLint info_log_size = 512;

    unsigned int vertex_shader = 0;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, nullptr);
    glCompileShader(vertex_shader);

    int vertex_success = 0;
    std::string info_log;
    info_log.reserve(info_log_size);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_success);
    if (vertex_success == 0)
    {
        glGetShaderInfoLog(vertex_shader, info_log_size, nullptr, info_log.begin().base());
        throw std::runtime_error(std::string("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n").append(info_log));
    }

    unsigned int fragment_shader = 0;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, nullptr);
    glCompileShader(fragment_shader);

    int fragment_success = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_success);
    if (fragment_success == 0)
    {
        glGetShaderInfoLog(fragment_shader, info_log_size, nullptr, info_log.begin().base());
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

void ShaderProgram::use() const
{
    glUseProgram(program);
}

auto ShaderProgram::get_uniform_location(std::string_view uniform) const -> GLint
{
    return glGetUniformLocation(program, uniform.begin());
}

auto shader_ncnn() -> ShaderProgram
{
    static auto program = ShaderProgram(0);
    static bool initialized = false;
    if (!initialized)
    {
        program = ShaderProgram(VERTEX_NCNN, FRAGMENT_WCNN);
        initialized = true;
    }
    return program;
}

auto shader_wcnn() -> ShaderProgram
{
    static auto program = ShaderProgram(0);
    static bool initialized = false;
    if (!initialized)
    {
        program = ShaderProgram(VERTEX_WCNN, FRAGMENT_WCNN);
        initialized = true;
    }
    return program;
}