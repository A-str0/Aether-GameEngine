#ifndef AETHERENGINE_RENDERING_ALLOCATION_H
#define AETHERENGINE_RENDERING_ALLOCATION_H

#include <vulkan/vulkan.hpp>

#include "buffer_context.hpp"

namespace AetherEngine::Rendering {
    struct Allocation {
        VkDeviceSize vertexOffset;
        VkDeviceSize indexOffset;
        VkDeviceSize vertexSize;
        VkDeviceSize indexSize;
        BufferContext vertexStaging;
        BufferContext indexStaging;
    };
}

#endif