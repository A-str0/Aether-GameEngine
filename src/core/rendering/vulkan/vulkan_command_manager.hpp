#ifndef AETHERENGINE_RENDERING_VULKANCOMMANDMANAGER_H
#define AETHERENGINE_RENDERING_VULKANCOMMANDMANAGER_H

#include <memory>

#include <vulkan/vulkan.hpp>

#include "vulkan_device_context.h"
#include "vulkan_swapchain_context.h"

namespace AetherEngine::Rendering {
    class VulkanCommandManager {
    public:
        static const uint8_t MAX_FRAMES_IN_FLIGHT = 2; // Renderer.h already has this const

        VulkanCommandManager(std::shared_ptr<VulkanDeviceContext> deviceContext_ptr, std::shared_ptr<VulkanSwapchainContext> swapchainContext_ptr);
        ~VulkanCommandManager();

        VulkanCommandManager(const VulkanCommandManager&) = delete;
        VulkanCommandManager& operator=(const VulkanCommandManager&) = delete;

        VulkanCommandManager(VulkanCommandManager&&) noexcept = default;
        VulkanCommandManager& operator=(VulkanCommandManager&&) noexcept = default;

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void recreateCommandBuffers();

        VkCommandPool getCommandPool() { return m_commandPool; }
        VkCommandPool getTransferCommandPool() { return m_transferCommandPool; }
        const std::vector<VkCommandBuffer>& getCommandBuffers() const { return m_commandBuffers; }

        const std::vector<VkSemaphore>& getImageAvailableSemaphores() const { return m_imageAvailableSemaphores; }
        const std::vector<VkSemaphore>& getRenderFinishedSemaphores() const { return m_renderFinishedSemaphores; }
        const std::vector<VkFence>& getInFlightFences() const { return m_inFlightFences; }
    private:
        std::shared_ptr<VulkanDeviceContext> m_deviceContext_ptr;
        std::shared_ptr<VulkanSwapchainContext> m_swapchainContext_ptr;

        void createTransferCommandPool();
        void createCommandPool();
        void createCommandBuffers();
        void createSyncObjects();

        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        VkCommandPool m_transferCommandPool = VK_NULL_HANDLE;

        std::vector<VkCommandBuffer> m_commandBuffers;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
    };
}

#endif
