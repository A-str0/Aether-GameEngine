#ifndef AETHERENGINE_RENDERING_RENDERER_H
#define AETHERENGINE_RENDERING_RENDERER_H

#include <vector>
#include <memory>

#include "vulkan_device_context.h"
#include "vulkan_swapchain_context.h"
#include <vulkan/vulkan.hpp>

#include "../objects/vertex.h" // TODO: change
#include "mesh_component.hpp"
#include "allocation.hpp"
#include "vulkan_buffer_manager.hpp"
#include "vulkan_command_manager.hpp"

namespace AetherEngine::Rendering {
    class Renderer {
    public:
        static const uint8_t MAX_FRAMES_IN_FLIGHT = 2;

        Renderer(VulkanDeviceContext& device, VulkanSwapchainContext& swapchain, std::shared_ptr<VulkanBufferManager> bufferManager_ptr, std::shared_ptr<VulkanCommandManager> commandManager_ptr);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) noexcept = default;
        Renderer& operator=(Renderer&&) noexcept = default;

        void drawFrame(std::vector<MeshComponent> meshes);

        void updateDescriptorSets(VkImageView imageView);
        VkDescriptorSet createDescriptorSet();
    private:
        void createRenderPass();
        void createRenderPass2();
        void createShaderModules();
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        
        void copyAllocation(Allocation& alloc);

        void recordCommandBuffer(std::vector<MeshComponent> meshes, VkCommandBuffer commandBuffer, uint32_t imageIndex);

        VulkanDeviceContext& m_deviceContext;
        VulkanSwapchainContext& m_swapchainContext;
        std::shared_ptr<VulkanBufferManager> m_bufferManager_ptr;
        std::shared_ptr<VulkanCommandManager> m_commandManager_ptr;

        VkShaderModule m_vertexShaderModule = VK_NULL_HANDLE;
        VkShaderModule m_fragmentShaderModule = VK_NULL_HANDLE;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;
        VkPipelineLayout m_graphicsPipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        
        std::vector<VkDescriptorSet> m_descriptorSets;

        // TODO: maybe not vectors???
        std::vector<VkFramebuffer> m_frameBuffers;
    };
}

#endif