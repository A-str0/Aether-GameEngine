#ifndef AETHERENGINE_RESOURCES_RESOURCEMANAGER_H
#define AETHERENGINE_RESOURCES_RESOURCEMANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "objects/texture_resource.h" // TODO: change
#include "../rendering/vulkan/vulkan_device_context.h" // TODO: change
#include <core/rendering/vulkan/vulkan_buffer_manager.hpp>
#include <core/rendering/vulkan/vulkan_command_manager.hpp>

namespace AetherEngine::ResourceManagment {
    class ResourceManager {
    public:
        ResourceManager(
            Rendering::VulkanDeviceContext& deviceContext, 
            Rendering::VulkanSwapchainContext& swapchainContext, 
            std::shared_ptr<Rendering::VulkanBufferManager> bufferManager_ptr,
            std::shared_ptr<Rendering::VulkanCommandManager> commandManager_ptr
        ) : 
            m_deviceContext(deviceContext),
            m_swapchainContext(swapchainContext),
            m_bufferManager_ptr(bufferManager_ptr),
            m_commandManager_ptr(commandManager_ptr)
        {} // TODO: change
        // ~ResourceManager(); 

        std::shared_ptr<Objects::TextureResource> loadTexture(std::string filename);
        void unloadTexture(u_char* pixels);
    private:
        Rendering::VulkanDeviceContext& m_deviceContext;
        Rendering::VulkanSwapchainContext& m_swapchainContext;
        std::shared_ptr<Rendering::VulkanBufferManager> m_bufferManager_ptr;
        std::shared_ptr<Rendering::VulkanCommandManager> m_commandManager_ptr;

        // TODO: change?
        std::unordered_map<std::string, std::weak_ptr<Objects::TextureResource>> m_textureCache;
        
        VkSampler createSampler();

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    };
}

#endif
