#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColour;
out vec4 vertex_colour;
uniform vec3 offset;
uniform mat4 rotation_mat;
uniform mat4 transform_mat;
void main() {
    vec3 rotated_position = (rotation_mat * vec4(aPos.xyz, 1.0f)).xyz;
    vec4 offset_position = vec4(rotated_position + offset, 1.0f);
    gl_Position = transform_mat * offset_position;
    vertex_colour = aColour;
}
