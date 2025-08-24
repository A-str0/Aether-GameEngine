#ifndef AETHERENGINE_RENDERING_OBJECTS_MATERIAL_H
#define AETHERENGINE_RENDERING_OBJECTS_MATERIAL_H

#include <vulkan/vulkan.hpp>

namespace AetherEngine::Rendering::Objects {
    class Material {
    public:
        Material() = default;
        Material(VkPipeline* graphicsPipline_ptr) : m_graphicsPipline_ptr(graphicsPipline_ptr) {}
        virtual ~Material() {
            delete m_graphicsPipline_ptr;
            m_graphicsPipline_ptr = nullptr;
        }
    private:
        VkPipeline* m_graphicsPipline_ptr;
        // TODO: add compute pipline
        // TODO: add descriptors pointer
    };
}

#endif