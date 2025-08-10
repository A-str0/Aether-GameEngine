#include "core/rendering/vulkan/vulkan_device_context.h"
#include "core/rendering/vulkan/vulkan_swapchain_context.h"
#include <vulkan/vulkan.hpp>
#include <vector>

#ifndef AETHERENGINE_RENDERING_RENDERER_H
#define AETHERENGINE_RENDERING_RENDERER_H

namespace AetherEngine::Rendering {
    class Renderer {
    public:
        Renderer(VulkanDeviceContext& device, VulkanSwapchainContext& swapchain, VkSurfaceKHR surface);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) noexcept = default;
        Renderer& operator=(Renderer&&) noexcept = default;

        void drawFrame();

    private:
        void createShaderModules();
        void createVertexBuffer();
        void createGraphicsPipeline();
        void createCommandBuffers();
        void createSyncObjects();

        VulkanDeviceContext& m_deviceContext;
        VulkanSwapchainContext& m_swapchainContext;

        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkShaderModule m_vertexShaderModule = VK_NULL_HANDLE;
        VkShaderModule m_fragmentShaderModule = VK_NULL_HANDLE;
        VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

        // TODO: maybe not vectors???
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
    };
}

#endif