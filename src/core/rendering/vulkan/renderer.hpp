#ifndef AETHERENGINE_RENDERING_RENDERER_H
#define AETHERENGINE_RENDERING_RENDERER_H

#include "vulkan_device_context.hpp"
#include "vulkan_swapchain_context.hpp"
#include <vulkan/vulkan.hpp>
#include <vector>

#include <rendering/objects/vertex.hpp>
#include <rendering/vulkan/objects/mesh.hpp>
#include "uniform_buffer_object.hpp"
#include <rendering/vulkan/memory_manager.hpp>

namespace AetherEngine::Rendering {
    class Renderer {
    public:
        Renderer(VulkanDeviceContext* deviceContext_ptr, VulkanSwapchainContext* swapchainContext_ptr, MemoryManager* memoryManager_ptr, VkSurfaceKHR surface);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) noexcept = default;
        Renderer& operator=(Renderer&&) noexcept = default;

        void drawFrame(const std::vector<Objects::Mesh*> meshes);
        void updateDescriptorSets(VkImageView imageView);

    private:
        void createRenderPass();
        void createRenderPass2();
        void createShaderModules(); // TODO: make external?
        void createGraphicsPipeline();
        // TODO: void createComputePipline();
        void createFramebuffers();
        void createSyncObjects();
        void createTextureSampler();
        // void createDescriptorPool();

        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const std::vector<Objects::Mesh*> meshes);

        // void createDescriptorSetLayout(); // TODO: move to material.hpp
        // void createDescriptorSets();      // TODO: move to mesh.hpp
        // void updateDescriptorSets(VkImageView imageView); // TODO: delete?

        VulkanDeviceContext* m_deviceContext_ptr;
        VulkanSwapchainContext* m_swapchainContext_ptr;
        MemoryManager* m_memoryManager_ptr;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkShaderModule m_vertexShaderModule = VK_NULL_HANDLE;   // TODO: make external?
        VkShaderModule m_fragmentShaderModule = VK_NULL_HANDLE; // TODO: make external?
        VkPipelineLayout m_graphicsPipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
        VkSampler m_textureSampler;

        // VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        // // TODO: smth
        // VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        // std::vector<VkDescriptorSet> m_descriptorSets;

        // TODO: maybe not vectors???
        std::vector<VkFramebuffer> m_frameBuffers;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
    };
}

#endif