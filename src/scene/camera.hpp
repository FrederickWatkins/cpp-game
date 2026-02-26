#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera
{
  public:
    Camera(glm::vec3 position, glm::quat rotation, float fov, float aspect_ratio, float clip_near, float clip_far);
    auto projection_mat() -> glm::mat4;
    auto pos() -> glm::vec3;
    void set_position(glm::vec3 position);
    void set_rotation(glm::quat rotation);
    void set_fov(float fov);
    void set_aspect_ratio(float aspect_ratio);
    void set_clip_near(float clip_near);
    void set_clip_far(float clip_far);
    void offset_position(glm::vec3 offset);
    void offset_rotation(glm::quat offset);
    void look_at(glm::vec3 target, glm::vec3 up_axis);
  private:
    glm::vec3 position;
    glm::quat rotation;
    float fov;
    float aspect_ratio;
    float clip_near;
    float clip_far;
};