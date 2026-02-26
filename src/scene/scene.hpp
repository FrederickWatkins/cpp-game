#pragma once

#include "camera.hpp"
#include "light.hpp"
#include "node.hpp"
#include <assimp/scene.h>
#include <memory>
#include <vector>

class Scene
{
  public:
    Scene(std::shared_ptr<Camera> camera, std::shared_ptr<Light> light);
    Scene(std::vector<std::shared_ptr<VirtualNode>> &nodes, std::shared_ptr<Camera> camera, std::shared_ptr<Light> light);
    void add_node(std::shared_ptr<VirtualNode> node);
    void draw();
  private:
    std::vector<std::shared_ptr<VirtualNode>> nodes;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Light> light;
};