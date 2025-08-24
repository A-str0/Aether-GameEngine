#ifndef AETHERENGINE_RESOURCES_RESOURCEMANAGER_H
#define AETHERENGINE_RESOURCES_RESOURCEMANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "objects/texture_resource.h" // TODO: change
#include "objects/ae_object.h" // TODO: change
#include "../rendering/vulkan/vulkan_device_context.h" // TODO: change
#include "../rendering/vulkan/renderer.h" // TODO: change

namespace AetherEngine::ResourceManagment {
    class ResourceManager {
    public:
        ResourceManager(
            Rendering::VulkanDeviceContext& deviceContext, 
            Rendering::VulkanSwapchainContext& swapchainContext, 
            Rendering::Renderer& renderer
        ) : 
            m_renderer(renderer), 
            m_swapchainContext(swapchainContext),
            m_deviceContext(deviceContext) 
        {} // TODO: change
        // ~ResourceManager(); 

        std::shared_ptr<Objects::TextureResource> loadTexture(std::string filename);
        void unloadTexture(u_char* pixels);

        // TODO: move onto the ECS
        std::shared_ptr<Objects::AE_Object> craeteObject(std::string name);
        void deleteObject(std::string name);
    private:
        Rendering::VulkanDeviceContext& m_deviceContext;
        Rendering::VulkanSwapchainContext& m_swapchainContext;
        Rendering::Renderer& m_renderer;

        std::unordered_map<std::string, std::weak_ptr<Objects::TextureResource>> m_textureCache;
        std::unordered_map<std::string, std::weak_ptr<Objects::AE_Object>> m_objectsCache;

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    };
}

#endif