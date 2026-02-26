#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

Camera::Camera(glm::vec3 position, glm::quat rotation, float fov, float aspect_ratio, float clip_near, float clip_far)
    : position(position), rotation(rotation), fov(fov), aspect_ratio(aspect_ratio), clip_near(clip_near),
      clip_far(clip_far)
{
}

auto Camera::pos() -> glm::vec3
{
    return position;
}

void Camera::set_position(glm::vec3 pos)
{
    this->position = pos;
}

void Camera::set_rotation(glm::quat rot)
{
    rotation = glm::normalize(rot);
}

void Camera::set_fov(float fov)
{
    this->fov = fov;
}
void Camera::set_aspect_ratio(float aspect_ratio)
{
    this->aspect_ratio = aspect_ratio;
}
void Camera::set_clip_near(float clip_near)
{
    this->clip_near = clip_near;
}
void Camera::set_clip_far(float clip_far)
{
    this->clip_far = clip_far;
}

void Camera::offset_position(glm::vec3 offset)
{
    position += offset;
}

void Camera::offset_rotation(glm::quat offset)
{
    rotation = glm::normalize(rotation * offset);
}

void Camera::look_at(glm::vec3 target, glm::vec3 up_axis)
{
    glm::vec3 direction = glm::normalize(target - position);

    rotation = glm::quatLookAt(direction, up_axis);
}

auto Camera::projection_mat() -> glm::mat4
{
    glm::mat4 view_rev_rot = glm::mat4_cast(glm::conjugate(rotation));
    glm::mat4 view_rev_pos = glm::translate(glm::mat4(1.0F), -position);

    glm::mat4 view = view_rev_rot * view_rev_pos;

    glm::mat4 projection = glm::perspective(glm::radians(fov), aspect_ratio, clip_near, clip_far);

    return projection * view;
}