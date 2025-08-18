#include <vulkan/vulkan.hpp>

#ifndef AETHERENGINE_QUEUEFAMILYINDICIESSTRUCT_H
#define AETHERENGINE_QUEUEFAMILYINDICIESSTRUCT_H

namespace AetherEngine {
    struct QueueFamilyIndices {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;
        uint32_t transferFamily = UINT32_MAX;
    };
}

#endif