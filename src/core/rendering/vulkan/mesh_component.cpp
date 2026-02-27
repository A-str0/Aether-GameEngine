#include "mesh_component.hpp"

namespace AetherEngine::Rendering {
    MeshComponent::MeshComponent(
        VkDeviceSize vertexOffset,
        VkDeviceSize indexOffset,
        uint32_t indexCount,
        std::shared_ptr<Material> material
    ) :
        m_vertexOffset(vertexOffset),
        m_indexOffset(indexOffset),
        m_indexCount(indexCount),
        m_material_ptr(material)
    { }

    MeshComponent::~MeshComponent() = default;

    void MeshComponent::updateUniformBuffer(const glm::mat4& viewProjMatrix) {
        // glm::mat4 modelMatrix = calculateModelMatrix();
        // glm::mat4 mvp = viewProjMatrix * modelMatrix;

        // memcpy(uniformBufferMapped, &mvp, sizeof(mvp));
    }
}
