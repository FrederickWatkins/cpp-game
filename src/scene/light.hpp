#pragma once

#include <glm/glm.hpp>

class Light
{
  public:
    auto pos() -> glm::vec3;
    auto colour() -> glm::vec3;
    auto intensities() -> glm::vec3;
};