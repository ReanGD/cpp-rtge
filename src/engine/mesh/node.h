#pragma once

#include <vector>

#include "engine/material/material.h"


class Mesh;
class PhysicalNode;
class Node {
public:
    Node() = default;
    ~Node() = default;

    void Add(const std::shared_ptr<Mesh>& mesh, const Material& material);
    void SetPhysicalNode(const std::shared_ptr<PhysicalNode>& node) noexcept;
    void SetModelMatrix(const glm::mat4& matrix) noexcept;

    void Update();
    uint32_t Draw(const std::shared_ptr<Camera>& camera) const;
    uint32_t DrawWithMaterial(const std::shared_ptr<Camera>& camera, const Material& material) const;

private:
    glm::mat4 m_matModel = glm::mat4(1.0f);
    glm::mat3 m_matNormal = glm::mat3(1.0f);
    std::shared_ptr<PhysicalNode> m_physicalNode = nullptr;

    std::vector<std::pair<std::shared_ptr<Mesh>, Material>> m_meshes;
};