#ifndef AETHERENGINE_RESOURCEMANAGMENT_OBJECTS_TEXTURERESOURCE_H
#define AETHERENGINE_RESOURCEMANAGMENT_OBJECTS_TEXTURERESOURCE_H

#include <vulkan/vulkan.hpp>

namespace AetherEngine::ResourceManagment::Objects {
    struct TextureResource
    {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView imageView;

        // TODO: change?
        void cleanup(VkDevice device) {
            vkDestroyImageView(device, imageView, nullptr);
            vkDestroyImage(device, image, nullptr);
            vkFreeMemory(device, memory, nullptr);
        }
    };
    
}

#endif