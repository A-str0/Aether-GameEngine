#ifndef AETHERENGINE_RESOURCEMANAGMENT_OBJECTS_TEXTURERESOURCE_H
#define AETHERENGINE_RESOURCEMANAGMENT_OBJECTS_TEXTURERESOURCE_H

#include <vulkan/vulkan.hpp>

namespace AetherEngine::ResourceManagment::Objects {
    struct TextureResource
    {
        std::shared_ptr<VkDevice> device_ptr;

        // TODO: change to pointers
        VkImage image;
        VkDeviceMemory memory;
        VkImageView imageView;
        VkSampler sampler;

        // TODO: change?
        void cleanup() {
            vkDestroyImageView(*device_ptr, imageView, nullptr);
            vkDestroyImage(*device_ptr, image, nullptr);
            vkFreeMemory(*device_ptr, memory, nullptr);
            vkDestroySampler(*device_ptr, sampler, nullptr);
        }
    };
    
}

#endif