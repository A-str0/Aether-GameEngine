#ifndef AETHERENGINE_RENDERING_BUFFERCONTEXT_H
#define AETHERENGINE_RENDERING_BUFFERCONTEXT_H

#include <memory>

#include <vulkan/vulkan.hpp>

namespace AetherEngine::Rendering {
    struct BufferContext {
        ~BufferContext() {
            if (buffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(*device_ptr.lock(), buffer, nullptr);
            }
            if (memory != VK_NULL_HANDLE) {
                vkFreeMemory(*device_ptr.lock(), memory, nullptr);
            }
        }

        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        VkDeviceSize currentOffset = 0;

        const std::weak_ptr<VkDevice> device_ptr;
    };
}

#endif