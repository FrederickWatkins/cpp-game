#pragma once
#include <glad/glad.h>
#include <string_view>

#include "fragment_shader.h" // IWYU pragma: keep
#include "vertex_shader.h"   // IWYU pragma: keep

class ShaderProgram
{
  public:
    ShaderProgram(const char *vertex_source, const char *fragment_source);
    void use();
    auto get_uniform_location(std::string_view uniform) -> GLuint;

  private:
    unsigned int program;
};
