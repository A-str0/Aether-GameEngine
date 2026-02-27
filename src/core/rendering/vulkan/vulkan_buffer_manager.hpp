#ifndef AETHERENGINE_RENDERING_VULKANBUFFERMANAGER_H
#define AETHERENGINE_RENDERING_VULKANBUFFERMANAGER_H

#include <core/rendering/objects/vertex.h>

#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "buffer_context.hpp"
#include "vulkan_command_manager.hpp"
#include "vulkan_device_context.h"

namespace AetherEngine::Rendering {
class VulkanBufferManager {
   public:
    VulkanBufferManager(std::shared_ptr<VulkanDeviceContext> deviceContext_ptr,
                        std::shared_ptr<VulkanCommandManager> commandManager_ptr);
    ~VulkanBufferManager();

    std::shared_ptr<BufferContext> getIndexBufferPtr() const { return m_indexBuffer_ptr; }
    std::shared_ptr<BufferContext> getVertexBufferPtr() const { return m_vertexBuffer_ptr; }

    std::shared_ptr<BufferContext> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                                VkMemoryPropertyFlags properties);
    void copyBuffer(BufferContext* srcBuffer, BufferContext* dstBuffer, VkBufferCopy copyRegion);
    struct MeshUploadResult {
        VkDeviceSize vertexOffset = 0;
        VkDeviceSize indexOffset = 0;
        uint32_t indexCount = 0;
    };
    MeshUploadResult uploadMesh(const std::vector<Rendering::Objects::Vertex>& vertices,
                                const std::vector<uint32_t>& indices);

   private:
    std::shared_ptr<VulkanDeviceContext> m_deviceContext_ptr;
    std::shared_ptr<VulkanCommandManager> m_commandManager_ptr;

    std::shared_ptr<BufferContext> m_vertexBuffer_ptr;
    std::shared_ptr<BufferContext> m_indexBuffer_ptr;

    VkDeviceSize m_alignment = 0;

    void createVertexBuffer();
    void createIndexBuffer();
    void ensureCapacity(std::shared_ptr<BufferContext>& bufferContext, VkDeviceSize requiredSize,
                        VkBufferUsageFlags usage);
};
}  // namespace AetherEngine::Rendering

#endif
