#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColour;
out vec4 vertex_colour;
uniform mat4 combined_transform_mat;
void main() {
    gl_Position = combined_transform_mat * vec4(aPos.xyz, 1.0f);
    vertex_colour = aColour;
}
