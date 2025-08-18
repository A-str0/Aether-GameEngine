#include "vulkan_device_context.h"
#include "vulkan_swapchain_context.h"
#include "../objects/vertex.h"
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
        void createRenderPass();
        void createRenderPass2();
        void createShaderModules();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createBuffers();
        void createTransferCommandPool();
        void createCommandPool();
        void createCommandBuffers();
        void createSyncObjects();

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

        VulkanDeviceContext& m_deviceContext;
        VulkanSwapchainContext& m_swapchainContext;

        VkShaderModule m_vertexShaderModule = VK_NULL_HANDLE;
        VkShaderModule m_fragmentShaderModule = VK_NULL_HANDLE;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkPipelineLayout m_graphicsPipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

        VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

        VkCommandPool m_commandPool = VK_NULL_HANDLE;
        VkCommandPool m_transferCommandPool = VK_NULL_HANDLE;

        // TODO: maybe not vectors???
        std::vector<VkFramebuffer> m_frameBuffers;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;

        // TODO: refactor
        const std::vector<Objects::Vertex> m_vertices = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        };
    };
}

#endif