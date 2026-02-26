#version 330 core
#define DEFINES
#ifdef VERTEX_COLOUR
in vec4 vertex_colour;
#endif
#ifdef LIGHTING
in vec3 light_dir;
in vec3 normal;
in vec3 frag_pos;
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform vec3 light_colour;
uniform vec3 intensities;
uniform vec3 view_pos;
uniform Material material;
#endif

out vec4 frag_colour;

void main() {
    frag_colour = vec4(1.0f);
    #ifdef VERTEX_COLOUR
    frag_colour *= vertex_colour;
    #endif
    #ifdef LIGHTING
    vec4 ambient_colour = vec4(material.ambient.xyz, 1.0f) * intensities.x;
    float diffuse = max(dot(normal, light_dir), 0.0);
    vec4 diffuse_colour = diffuse * intensities.y * vec4(material.diffuse, 1.0f);
    vec3 view_dir = view_pos - frag_pos;
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec4 specular_colour = vec4(spec * intensities.z * material.specular, 1.0f);
    frag_colour *= vec4(light_colour, 1.0f);
    frag_colour *= ambient_colour + diffuse_colour + specular_colour;
    #endif
}
