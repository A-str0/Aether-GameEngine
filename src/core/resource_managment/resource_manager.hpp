#ifndef AETHERENGINE_RESOURCES_RESOURCEMANAGER_H
#define AETHERENGINE_RESOURCES_RESOURCEMANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include <resource_managment/objects/texture_resource.hpp>
#include <rendering/vulkan/vulkan_device_context.hpp>
#include <rendering/vulkan/renderer.hpp>
#include <rendering/vulkan/objects/mesh.hpp>

namespace AetherEngine::ResourceManagment {
    class ResourceManager {
    public:
        ResourceManager(
            Rendering::VulkanDeviceContext* deviceContext_ptr, 
            Rendering::VulkanSwapchainContext* swapchainContext_ptr, 
            Rendering::MemoryManager* memoryManager_ptr
        ) : 
            m_swapchainContext_ptr(swapchainContext_ptr),
            m_deviceContext_ptr(deviceContext_ptr), 
            m_memoryManager_ptr(memoryManager_ptr)
        { } // TODO: change
        // ~ResourceManager(); 

        std::shared_ptr<Objects::TextureResource> loadTexture(std::string filename);
        void unloadTexture(u_char* pixels);

        Rendering::VulkanDeviceContext* m_deviceContext_ptr;
        Rendering::VulkanSwapchainContext* m_swapchainContext_ptr;
        Rendering::MemoryManager* m_memoryManager_ptr;

        std::unordered_map<std::string, std::weak_ptr<Objects::TextureResource>> m_textureCache;

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    };
}

#endif