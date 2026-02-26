#version 330 core
#define DEFINES
#ifdef VERTEX_COLOUR
in vec4 vertex_colour;
#endif
#ifdef LIGHTING
in vec3 normal;
in vec3 frag_pos;
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform vec3 light_pos;
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
    vec3 light_dir = normalize(light_pos - frag_pos);
    float light_attenuation = 150.0f * pow(length(light_pos - frag_pos), -0.7f);
    // float light_attenuation = 1.0f;
    vec4 ambient_colour = vec4(material.ambient.xyz, 1.0f) * intensities.x;
    float diffuse = light_attenuation * max(dot(normalize(normal), normalize(light_dir)), 0.0);
    vec4 diffuse_colour = diffuse * intensities.y * vec4(material.diffuse, 1.0f);
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-normalize(light_dir), normalize(normal));
    float spec = light_attenuation * pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec4 specular_colour = vec4(spec * intensities.z * material.specular, 1.0f);
    frag_colour *= vec4(light_colour, 1.0f);
    frag_colour *= ambient_colour + diffuse_colour + specular_colour;
    #endif
}
