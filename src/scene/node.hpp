#pragma once

#include "../shader/shader.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include <memory>
#include <optional>
#include <type_traits>

struct MaterialValues
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

template <bool has_lighting> struct NodeValues
{
    glm::mat4 transform_mat;
    [[no_unique_address]] std::conditional_t<has_lighting, MaterialValues, empty_material> material;
};

class VirtualNode
{
  public:
    VirtualNode() = default;
    VirtualNode(const VirtualNode &) = default;
    VirtualNode(VirtualNode &&) = delete;
    auto operator=(const VirtualNode &) -> VirtualNode & = default;
    auto operator=(VirtualNode &&) -> VirtualNode & = delete;
    virtual ~VirtualNode() = default;

    virtual void draw(Camera &camera, Light &light) = 0;
    virtual void set_transform(glm::mat4 transform) = 0;
};

template <bool NDC, bool has_colour, bool has_lighting, size_t num_tex_coords> class Node : public VirtualNode
{
    using Mesh = Mesh<has_colour, has_lighting, num_tex_coords>;
    using Shader = ShaderProgram<NDC, has_colour, has_lighting, num_tex_coords>;

  public:
    Node(
        std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader, glm::mat4 transform_mat,
        std::optional<MaterialValues> material = std::nullopt
    )
        : mesh(mesh), shader(shader), uniforms(shader->get_uniforms())
    {
        values.transform_mat = transform_mat;
        if constexpr (has_lighting)
        {
            assert(material != std::nullopt);
            values.material = material;
        }
    }
    void draw(Camera &camera, Light &light) override
    {
        shader->use();
        mesh->use();
        uniforms.transform_mat.set(values.transform_mat);
        if constexpr (!NDC)
        {
            uniforms.projection_mat.set(camera.projection_mat());
        }
        if constexpr (has_lighting)
        {
            uniforms.material.ambient.set(values.material.ambient);
            uniforms.material.diffuse.set(values.material.diffuse);
            uniforms.material.specular.set(values.material.specular);
            uniforms.material.shininess.set(values.material.shininess);
            uniforms.light_pos.set(light.pos());
            uniforms.light_colour.set(light.colour());
            uniforms.intensities.set(light.intensities());
            uniforms.view_pos = camera.pos();
        }
        mesh->draw();
    }
    void set_transform(glm::mat4 transform_mat) override
    {
        values.transform_mat = transform_mat;
    }

  private:
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Shader> shader;
    ShaderUniforms<NDC, has_lighting> uniforms;
    NodeValues<has_lighting> values;
};
