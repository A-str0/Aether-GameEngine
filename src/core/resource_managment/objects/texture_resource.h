#ifndef AETHERENGINE_RESOURCEMANAGMENT_OBJECTS_TEXTURERESOURCE_H
#define AETHERENGINE_RESOURCEMANAGMENT_OBJECTS_TEXTURERESOURCE_H

#include <vulkan/vulkan.hpp>

namespace AetherEngine::ResourceManagment::Objects {
    struct TextureResource {
        VkDevice device = VK_NULL_HANDLE;
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;

        ~TextureResource() {
            cleanup();
        }

        void cleanup() {
            if (device == VK_NULL_HANDLE) {
                return;
            }
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(device, imageView, nullptr);
                imageView = VK_NULL_HANDLE;
            }
            if (image != VK_NULL_HANDLE) {
                vkDestroyImage(device, image, nullptr);
                image = VK_NULL_HANDLE;
            }
            if (memory != VK_NULL_HANDLE) {
                vkFreeMemory(device, memory, nullptr);
                memory = VK_NULL_HANDLE;
            }
            if (sampler != VK_NULL_HANDLE) {
                vkDestroySampler(device, sampler, nullptr);
                sampler = VK_NULL_HANDLE;
            }
        }
    };
}

#endif
