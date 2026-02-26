#version 330 core
#define DEFINES
layout(location = 0) in vec3 aPos;
#ifdef VERTEX_COLOUR
layout(location = 1) in vec4 aColour;
out vec4 vertex_colour;
#endif
#ifdef TEXTURE_COORDS_1D
layout(location = 2) in float aTexCoords;
#endif
#ifdef TEXTURE_COORDS_2D
layout(location = 2) in vec2 aTexCoords;
#endif
#ifdef TEXTURE_COORDS_3D
layout(location = 2) in vec3 aTexCoords;
#endif
#ifdef LIGHTING
layout(location = 3) in vec3 aNormals;
out vec3 normal;
out vec3 frag_pos;
#endif
uniform mat4 translate_mat;
#ifndef NDC
uniform mat4 transform_mat;
#endif
void main() {
    gl_Position = vec4(aPos.xyz, 1.0f);
    gl_Position = translate_mat * gl_Position;
    #ifndef NDC
    gl_Position = transform_mat * gl_Position;
    #endif
    #ifdef VERTEX_COLOUR
    vertex_colour = aColour;
    #endif
    #ifdef LIGHTING
    normal = normalize(mat3(translate_mat) * aNormals);
    frag_pos = (translate_mat * vec4(aPos.xyz, 1.0f)).xyz;
    #endif
}
