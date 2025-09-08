#ifndef AETHERENGINE_RENDERING_VULKANBUFFERMANAGER_H
#define AETHERENGINE_RENDERING_VULKANBUFFERMANAGER_H

#include <memory>

#include <vulkan/vulkan.hpp>

#include "buffer_context.hpp"
#include "vulkan_device_context.h"
#include "vulkan_command_manager.hpp"
#include "allocation.hpp"
#include <core/rendering/objects/vertex.h>

namespace AetherEngine::Rendering {
    class VulkanBufferManager {
    public:
        VulkanBufferManager(std::shared_ptr<VulkanDeviceContext> deviceContext_ptr, std::shared_ptr<VulkanCommandManager> commandManager_ptr);
        ~VulkanBufferManager();

        std::shared_ptr<BufferContext> getIndexBufferPtr() const { return m_indexBuffer_ptr; }
        std::shared_ptr<BufferContext> getVertexBufferPtr() const { return m_vertexBuffer_ptr; }
        
        std::shared_ptr<BufferContext> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        void copyBuffer(BufferContext* srcBuffer, BufferContext* dstBuffer, VkBufferCopy copyRegion);
        void resizeBuffer(std::weak_ptr<BufferContext> bufferContext, VkDeviceSize newSize, VkBufferUsageFlags usage);

        Allocation prepareAllocation(const std::vector<Rendering::Objects::Vertex>& vertices, const std::vector<uint32_t>& indices);
    private:   
        std::shared_ptr<VulkanDeviceContext> m_deviceContext_ptr;
        std::shared_ptr<VulkanCommandManager> m_commandManager_ptr;

        std::shared_ptr<BufferContext> m_vertexBuffer_ptr;
        std::shared_ptr<BufferContext> m_indexBuffer_ptr;
        std::vector<std::shared_ptr<BufferContext>> m_uniformBuffersPtrs;
        std::vector<void*> m_uniformBuffersMapped;

        VkDeviceSize alignment = 0;

        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffer();
    };
}

#endif