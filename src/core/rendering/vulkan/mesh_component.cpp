#include "mesh_component.hpp"

namespace AetherEngine::Rendering {
    MeshComponent::MeshComponent(
        const std::vector<AetherEngine::Rendering::Objects::Vertex> vertices, 
        const std::vector<uint16_t> indices, 
        uint32_t vertexOffset, 
        uint32_t indexOffset,
        std::shared_ptr<Material> material
    ) : 
        m_vertices(vertices), 
        m_indices(indices), 
        m_vertexOffset(vertexOffset), 
        m_indexOffset(indexOffset),
        m_material_ptr(material)
    { }

    MeshComponent::~MeshComponent() {
        
    }

    void MeshComponent::updateUniformBuffer(const glm::mat4& viewProjMatrix) {
        // glm::mat4 modelMatrix = calculateModelMatrix();
        // glm::mat4 mvp = viewProjMatrix * modelMatrix;

        // memcpy(uniformBufferMapped, &mvp, sizeof(mvp));
    }
}