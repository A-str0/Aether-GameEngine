#ifndef AETHERENGINE_QUEUEFAMILYINDICIESSTRUCT_H
#define AETHERENGINE_QUEUEFAMILYINDICIESSTRUCT_H

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace AetherEngine {
    struct QueueFamilyIndices {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        uint32_t transferFamily = UINT32_MAX;
    };

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
}

#endif