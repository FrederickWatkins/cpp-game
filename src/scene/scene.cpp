#include "scene.hpp"

Scene::Scene(std::shared_ptr<Camera> camera, std::shared_ptr<Light> light) : nodes({}), camera(std::move(camera)), light(std::move(light))
{
}

Scene::Scene(std::vector<std::shared_ptr<VirtualNode>> &nodes, std::shared_ptr<Camera> camera, std::shared_ptr<Light> light)
    : nodes(nodes), camera(std::move(camera)), light(std::move(light))
{
}

void Scene::add_node(std::shared_ptr<VirtualNode> node)
{
    nodes.push_back(node);
}

void Scene::draw()
{
    for (auto &node : nodes)
    {
        node->draw(*camera, *light);
    }
}