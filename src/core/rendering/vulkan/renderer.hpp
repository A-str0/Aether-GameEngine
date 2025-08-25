#ifndef AETHERENGINE_RENDERING_RENDERER_H
#define AETHERENGINE_RENDERING_RENDERER_H

#include "vulkan_device_context.hpp"
#include "vulkan_swapchain_context.hpp"
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

#include <rendering/objects/vertex.hpp>
#include <rendering/vulkan/objects/model.hpp>
#include "uniform_buffer_object.hpp"

namespace AetherEngine::Rendering {
    class Renderer {
    public:
        Renderer(VulkanDeviceContext& device, VulkanSwapchainContext& swapchain, VkSurfaceKHR surface);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) noexcept = default;
        Renderer& operator=(Renderer&&) noexcept = default;

        void drawFrame(const std::vector<std::shared_ptr<Objects::Model>>& models);
        void updateDescriptorSets(VkImageView imageView);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    private:
        void createRenderPass();
        void createRenderPass2();
        void createShaderModules();
        void createGraphicsPipeline();
        void createFramebuffers();
        // void createVertexBuffer();
        // void createIndexBuffer();
        void createUniformBuffers();
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();
        void createTransferCommandPool();
        void createCommandPool();
        void createCommandBuffers();
        void createSyncObjects();
        void createTextureSampler();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const std::vector<std::shared_ptr<Objects::Model>>& models);
        void updateUniformBuffer(uint32_t currentImage);

        VulkanDeviceContext& m_deviceContext;
        VulkanSwapchainContext& m_swapchainContext;

        VkShaderModule m_vertexShaderModule = VK_NULL_HANDLE;
        VkShaderModule m_fragmentShaderModule = VK_NULL_HANDLE;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkPipelineLayout m_graphicsPipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
        VkSampler m_textureSampler;

        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;

        // TODO: for performance i should use on VkBuffer for Verticies and offset for this
        // It is even possible to reuse the same chunk of memory for multiple resources if 
        // they are not used during the same render operations, provided that their data is refreshed.
        // It calls Aliasing
        VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
        VkBuffer m_indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;
        std::vector<void*> m_uniformBuffersMapped;

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
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };
        const std::vector<uint16_t> m_indices = {
            0, 1, 2, 2, 3, 0
        };
    };
}

#endif