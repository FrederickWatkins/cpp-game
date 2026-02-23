#pragma once
#include <glm/glm.hpp>

class Camera
{
  public:
    Camera(
        const glm::vec3 &pos, const glm::vec3 &view_normal, float fov_degrees, float aspect_ratio, float near_clip,
        float far_clip, glm::vec3 &up_vector
    )
        : pos(pos), view_normal(view_normal), fov_degrees(fov_degrees), aspect_ratio(aspect_ratio),
          near_clip(near_clip), far_clip(far_clip), up_vector(up_vector) {};

    glm::vec3 pos;
    glm::vec3 view_normal;
    float fov_degrees;
    float aspect_ratio;
    float near_clip;
    float far_clip;
    glm::vec3 up_vector;
};