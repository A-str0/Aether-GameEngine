#ifndef AETHERENGINE_RESOURCES_RESOURCEMANAGER_H
#define AETHERENGINE_RESOURCES_RESOURCEMANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include <resource_managment/objects/texture_resource.hpp>
#include <rendering/vulkan/vulkan_device_context.hpp>
#include <rendering/vulkan/renderer.hpp>
#include <rendering/vulkan/objects/model.hpp>

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

        // Model management
        std::shared_ptr<Rendering::Objects::Model> createQuadModel();
        void addModel(std::shared_ptr<Rendering::Objects::Model> model);
        void removeModel(std::shared_ptr<Rendering::Objects::Model> model);
        const std::vector<std::shared_ptr<Rendering::Objects::Model>>& getModels() const;
    private:
        Rendering::VulkanDeviceContext& m_deviceContext;
        Rendering::VulkanSwapchainContext& m_swapchainContext;
        Rendering::Renderer& m_renderer;

        std::unordered_map<std::string, std::weak_ptr<Objects::TextureResource>> m_textureCache;
        std::vector<std::shared_ptr<Rendering::Objects::Model>> m_models;

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    };
}

#endif