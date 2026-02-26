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
out vec3 light_dir;
out vec3 normal;
out vec3 frag_pos;
#endif
#ifndef NDC
uniform mat4 combined_transform_mat;
#endif
void main() {
    #ifdef NDC
    gl_Position = vec4(aPos.xyz, 1.0f);
    #else
    gl_Position = combined_transform_mat * vec4(aPos.xyz, 1.0f);
    #endif
    #ifdef VERTEX_COLOUR
    vertex_colour = aColour;
    #endif
}
