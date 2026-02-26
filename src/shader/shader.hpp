#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string_view>

class ShaderProgram
{
  public:
    explicit ShaderProgram(unsigned int program) : program(program) {};
    ShaderProgram(const char *vertex_source, const char *fragment_source);
    void use() const;
    [[nodiscard]] auto get_uniform_location(std::string_view uniform) const -> GLint;

  private:
    unsigned int program;
};

// NOLINTBEGIN
struct alignas(16) Material
{
    alignas(16) glm::vec3 ambient;
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
    float shininess;
    float padding[3];
};
// NOLINTEND
