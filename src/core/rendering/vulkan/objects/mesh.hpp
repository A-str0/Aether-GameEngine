#ifndef AETHERENGINE_RENDERING_OBJECTS_MODEL_H
#define AETHERENGINE_RENDERING_OBJECTS_MODEL_H

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "material.hpp"
#include <src/core/rendering/objects/vertex.hpp>

namespace AetherEngine::Rendering::Objects {
    class Mesh {
    public:
        Mesh(
            std::vector<Vertex> _vertices, 
            std::vector<uint16_t> _indices,
            Material* material_ptr
        ) : 
            // vertices(_vertices),
            // indices(_indices),
            m_material_ptr(material_ptr) 
        { }
        virtual ~Mesh() {
            // Note: Buffer destruction should be handled by the resource manager
            delete m_material_ptr;
            m_material_ptr = nullptr;
        }

        const std::vector<Vertex> vertices;
        const std::vector<uint16_t> indices;

        Material* getMaterial() const { return m_material_ptr; }
        
        void setVertexOffset(uint16_t offset) { vertexOffset = offset; }
        void setIndexOffset(uint16_t offset) { indexOffset = offset; }
        
        uint16_t getVertexOffset() const { return vertexOffset; }
        uint16_t getIndexOffset() const { return indexOffset; }
    private:
        Material* m_material_ptr;

        uint16_t vertexOffset = 0;
        uint16_t indexOffset = 0;
    };
}

#endif
