#ifndef AETHERENGINE_RENDERING_OBJECTS_MODEL_H
#define AETHERENGINE_RENDERING_OBJECTS_MODEL_H

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "material.hpp"
#include <src/core/rendering/objects/vertex.hpp>

namespace AetherEngine::Rendering::Objects {
    class Model {
    public:
        Model(
            VkBuffer vertexBuffer, 
            VkBuffer indexBuffer, 
            std::vector<Vertex> _vertices, 
            std::vector<uint16_t> _indices,
            Material* material_ptr,
            glm::mat4 transform
        ) : 
            m_vertexBuffer(vertexBuffer), 
            m_indexBuffer(indexBuffer),
            vertices(_vertices),
            indices(_indices),
            m_material_ptr(material_ptr),
            m_transform(transform) { 
        }
        virtual ~Model() {
            // Note: Buffer destruction should be handled by the resource manager
            delete m_material_ptr;
            m_material_ptr = nullptr;
        }

        const std::vector<Vertex> vertices;
        const std::vector<uint16_t> indices;

        Material* getMaterial() const { return m_material_ptr; }
        glm::mat4 getTransform() const { return m_transform; }
        VkBuffer getVertexBuffer() const { return m_vertexBuffer; }
        VkBuffer getIndexBuffer() const { return m_indexBuffer; }

    private:
        VkBuffer m_vertexBuffer;
        VkBuffer m_indexBuffer;
        Material* m_material_ptr;
        glm::mat4 m_transform;
    };
}

#endif
