#include "shader.h"
#include <stdexcept>
#include <string>

ShaderProgram::ShaderProgram(const char *vertex_source, const char *fragment_source)
{
    unsigned int vertex_shader = 0;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, nullptr);
    glCompileShader(vertex_shader);

    int vertex_success = 0;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_success);
    if (vertex_success == 0)
    {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, infoLog);
        throw std::runtime_error(std::string("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n").append(infoLog));
    }

    unsigned int fragment_shader = 0;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, nullptr);
    glCompileShader(fragment_shader);

    int fragment_success = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_success);
    if (fragment_success == 0)
    {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, infoLog);
        throw std::runtime_error(std::string("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n").append(infoLog));
    }

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int link_success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &link_success);
    if (link_success == 0)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        throw std::runtime_error(std::string("ERROR::SHADER::LINK_FAILED\n").append(infoLog));
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void ShaderProgram::use()
{
    glUseProgram(program);
}

auto ShaderProgram::get_uniform_location(std::string_view uniform) -> GLuint
{
    return glGetUniformLocation(program, uniform.begin());
}
