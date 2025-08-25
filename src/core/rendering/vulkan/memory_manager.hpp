#ifndef AETHERENGINE_RENDERING_MEMORYMANAGER_H
#define AETHERENGINE_RENDERING_MEMORYMANAGER_H

#include <vulkan/vulkan.hpp>

#include <rendering/vulkan/vulkan_device_context.hpp>
#include <rendering/vulkan/vulkan_swapchain_context.hpp>
#include <rendering/vulkan/uniform_buffer_object.hpp>
#include <rendering/vulkan/objects/mesh.hpp>

namespace AetherEngine::Rendering {
    static const u_short MAX_FRAMES_IN_FLIGHT = 2; // TODO: change

    class MemoryManager {
    public:
        // MemoryManager(Renderer* renderer_ptr);
        MemoryManager(VulkanDeviceContext* deviceContext_ptr, VulkanSwapchainContext* swapchainContext_ptr);
        // MemoryManager(VulkanDeviceContext* deviceContext_ptr);
        ~MemoryManager() = default;

        MemoryManager(const MemoryManager&) = delete;
        MemoryManager& operator=(const MemoryManager&) = delete;

        MemoryManager(MemoryManager&&) noexcept = default;
        MemoryManager& operator=(MemoryManager&&) noexcept = default;

        const VkBuffer getVertexBuffer() { return m_vertexBuffer; }
        const VkBuffer getIndexBuffer() { return m_indexBuffer; }
        const std::vector<VkCommandBuffer> getCommandBuffers() { return m_commandBuffers; }
        const std::vector<VkBuffer> getUniformBuffers() { return m_uniformBuffers; }

        const std::vector<VkDescriptorSet> getDescriptorSets() { return m_descriptorSets; }
        const VkDescriptorSetLayout* getDescriptorSetLayout() { return &m_descriptorSetLayout; }

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void uploadMesh(const AetherEngine::Rendering::Objects::Mesh& mesh);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        void updateUniformBuffer(uint32_t currentImage);
    private:
        VulkanDeviceContext* m_deviceContext_ptr;
        VulkanSwapchainContext* m_swapchainContext_ptr;

        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffer();

        void createTransferCommandPool();

        void createCommandPool();
        void createCommandBuffers();
        
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();      // TODO: move to mesh.hpp

        // TODO: move to Scene object?
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

        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;

        // TODO: maybe not vectors???
        std::vector<VkCommandBuffer> m_commandBuffers;
    };
}

#endif