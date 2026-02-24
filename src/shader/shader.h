#pragma once
#include <glad/glad.h>
#include <string_view>

class ShaderProgram
{
  public:
    ShaderProgram(unsigned int program) : program(program) {};
    ShaderProgram(const char *vertex_source, const char *fragment_source);
    void use();
    auto get_uniform_location(std::string_view uniform) -> GLuint;

  private:
    unsigned int program;
};

/// Shader from NDC space, with colour, no texture mapping, no lighting
auto shader_ncnn() -> ShaderProgram;

/// Shader from world space, with colour, no texture mapping, no lighting
auto shader_wcnn() -> ShaderProgram;
