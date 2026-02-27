#ifndef AETHERENGINE_RENDERING_BUFFERCONTEXT_H
#define AETHERENGINE_RENDERING_BUFFERCONTEXT_H

#include <vulkan/vulkan.hpp>
#include <utility>

namespace AetherEngine::Rendering {
    struct BufferContext {
        BufferContext() = default;
        BufferContext(VkDevice deviceHandle, VkBuffer bufferHandle, VkDeviceMemory memoryHandle, VkDeviceSize bufferSize) 
            : device(deviceHandle), buffer(bufferHandle), memory(memoryHandle), size(bufferSize) {}

        BufferContext(const BufferContext&) = delete;
        BufferContext& operator=(const BufferContext&) = delete;

        BufferContext(BufferContext&& other) noexcept {
            *this = std::move(other);
        }

        BufferContext& operator=(BufferContext&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            destroy();

            device = other.device;
            buffer = other.buffer;
            memory = other.memory;
            size = other.size;
            currentOffset = other.currentOffset;

            other.device = VK_NULL_HANDLE;
            other.buffer = VK_NULL_HANDLE;
            other.memory = VK_NULL_HANDLE;
            other.size = 0;
            other.currentOffset = 0;
            return *this;
        }

        ~BufferContext() {
            destroy();
        }

        void destroy() {
            if (device == VK_NULL_HANDLE) {
                return;
            }
            if (buffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(device, buffer, nullptr);
                buffer = VK_NULL_HANDLE;
            }
            if (memory != VK_NULL_HANDLE) {
                vkFreeMemory(device, memory, nullptr);
                memory = VK_NULL_HANDLE;
            }
        }

        VkDevice device = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        VkDeviceSize currentOffset = 0;
    };
}

#endif
