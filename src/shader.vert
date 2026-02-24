#version 330 core
layout(location = 0) in vec3 aPos;
out vec4 vertexColour;
uniform vec4 ourColour;
uniform vec3 offset;
void main() {
    gl_Position = vec4((aPos.xyz + offset), 1.0);
    vertexColour = ourColour;
}
