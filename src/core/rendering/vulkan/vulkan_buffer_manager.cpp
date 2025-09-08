#include "vulkan_buffer_manager.hpp"

#include "renderer.h"
#include "uniform_buffer_object.hpp"

namespace AetherEngine::Rendering {
    VulkanBufferManager::VulkanBufferManager(
        std::shared_ptr<VulkanDeviceContext> deviceContext_ptr, 
        std::shared_ptr<VulkanCommandManager> commandManager_ptr
    ) :
        m_deviceContext_ptr(deviceContext_ptr),
        m_commandManager_ptr(commandManager_ptr)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(deviceContext_ptr->getPhysicalDevice(), &props);
        alignment = std::max(props.limits.minStorageBufferOffsetAlignment, props.limits.minUniformBufferOffsetAlignment);

        createIndexBuffer();
        createVertexBuffer();
        // createUniformBuffer();
    }

    VulkanBufferManager::~VulkanBufferManager() {
        // for (size_t i = 0; i < VulkanCommandManager::MAX_FRAMES_IN_FLIGHT; i++) {
        //     vkDestroyBuffer(m_deviceContext_ptr->getDevice(), m_uniformBuffersPtrs[i], nullptr);
        //     vkFreeMemory(m_deviceContext_ptr->getDevice(), m_uniformBuffersPtrs[i], nullptr);
        // }

        // vkDestroyBuffer(m_deviceContext_ptr->getDevice(), m_vertexBuffer, nullptr);
        // vkFreeMemory(m_deviceContext_ptr->getDevice(), m_vertexBufferMemory, nullptr);
        // vkDestroyBuffer(m_deviceContext_ptr->getDevice(), m_indexBuffer, nullptr);
        // vkFreeMemory(m_deviceContext_ptr->getDevice(), m_indexBufferMemory, nullptr);
    }

    std::shared_ptr<BufferContext> VulkanBufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceMemory memory{VK_NULL_HANDLE};
        
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.flags = 0; // TODO
        bufferInfo.pNext = nullptr; // TODO
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_deviceContext_ptr->getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer!");
        }

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(m_deviceContext_ptr->getDevice(), buffer, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = m_deviceContext_ptr->getMemoryType(memoryRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_deviceContext_ptr->getDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        if (vkBindBufferMemory(m_deviceContext_ptr->getDevice(), buffer, memory, 0) != VK_SUCCESS) {
            throw std::runtime_error("Failed to bind buffer memory!");
        }

        return std::make_shared<BufferContext>(BufferContext{
            buffer, memory, size, 0, std::make_shared<VkDevice>(m_deviceContext_ptr->getDevice())
        });
    }

    void VulkanBufferManager::copyBuffer(BufferContext* srcBuffer, BufferContext* dstBuffer, VkBufferCopy copyRegion) {
        VkCommandBuffer commandBuffer = m_commandManager_ptr->beginSingleTimeCommands();

        vkCmdCopyBuffer(commandBuffer, srcBuffer->buffer, dstBuffer->buffer, 1, &copyRegion);

        m_commandManager_ptr->endSingleTimeCommands(commandBuffer);
    }

    void VulkanBufferManager::resizeBuffer(std::weak_ptr<BufferContext> bufferContext, VkDeviceSize newSize, VkBufferUsageFlags usage) {
        std::shared_ptr<BufferContext> bufferContext_ptr = bufferContext.lock();

        if (newSize <= bufferContext_ptr->size) return;

        std::shared_ptr<BufferContext> newBuffer = createBuffer(newSize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (bufferContext_ptr->size > 0) {
            copyBuffer(bufferContext_ptr.get(), newBuffer.get(), VkBufferCopy{.srcOffset = 0, .dstOffset = 0, .size = bufferContext_ptr->size});
        }

        vkDestroyBuffer(m_deviceContext_ptr->getDevice(), bufferContext_ptr->buffer, nullptr);
        vkFreeMemory(m_deviceContext_ptr->getDevice(), bufferContext_ptr->memory, nullptr);

        bufferContext_ptr->buffer = newBuffer->buffer;
        bufferContext_ptr->memory = newBuffer->memory;
        bufferContext_ptr->size = newSize;

        newBuffer->buffer = VK_NULL_HANDLE;
        newBuffer->memory = VK_NULL_HANDLE;
    }

    void VulkanBufferManager::createVertexBuffer() {
        m_vertexBuffer_ptr = createBuffer(
            16, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
    }

    void VulkanBufferManager::createIndexBuffer() {
        m_indexBuffer_ptr = createBuffer(
            16, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
    }

    void VulkanBufferManager::createUniformBuffer() {
        VkDeviceSize bufferSize = ((sizeof(UniformBufferObject) + alignment - 1) / alignment) * alignment;

        m_uniformBuffersPtrs.resize(VulkanCommandManager::MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMapped.resize(VulkanCommandManager::MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < VulkanCommandManager::MAX_FRAMES_IN_FLIGHT; i++) {

            m_uniformBuffersPtrs[i] = createBuffer(
                bufferSize, 
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );

            vkMapMemory(m_deviceContext_ptr->getDevice(), m_uniformBuffersPtrs[i]->memory, 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
        }
    }

    Allocation VulkanBufferManager::prepareAllocation(const std::vector<Rendering::Objects::Vertex>& vertices, const std::vector<uint32_t>& indices) {
        VkDeviceSize vertexSize = vertices.size() * sizeof(Rendering::Objects::Vertex);
        VkDeviceSize indexSize = indices.size() * sizeof(uint32_t);

        vertexSize = ((vertexSize + alignment - 1) / alignment) * alignment;
        indexSize = ((indexSize + alignment - 1) / alignment) * alignment;

        if (m_vertexBuffer_ptr->currentOffset + vertexSize > m_vertexBuffer_ptr->size) {
            resizeBuffer(m_vertexBuffer_ptr, std::max(m_vertexBuffer_ptr->size * 2, m_vertexBuffer_ptr->currentOffset + vertexSize),
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        }
        if (m_indexBuffer_ptr->currentOffset + indexSize > m_indexBuffer_ptr->size) {
            resizeBuffer(m_indexBuffer_ptr, std::max(m_indexBuffer_ptr->size * 2, m_indexBuffer_ptr->currentOffset + indexSize),
                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        }
        // if (nextUBOIndex * alignment + sizeof(PerObjectUBO) > dynamicUBOSize) {
        //     VkDeviceSize newSize = std::max(dynamicUBOSize * 2, (nextUBOIndex + 1) * alignment);
        //     BufferContext newContext = createBuffer(newSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        //                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        //     void* newMapped;
        //     vkMapMemory(vkContext.device, newContext.memory.get(), 0, newSize, 0, &newMapped);
        //     memcpy(newMapped, mappedDynamicUBO, dynamicUBOSize);
        //     vkUnmapMemory(vkContext.device, dynamicUBO.memory.get());
        //     dynamicUBO = std::move(newContext);
        //     mappedDynamicUBO = newMapped;
        //     dynamicUBOSize = newSize;
        // }

        auto vertexStaging = createBuffer(vertexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vertexStaging->size = vertexSize;

        auto indexStaging = createBuffer(indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        indexStaging->size = indexSize;

        void* data;
        vkMapMemory(m_deviceContext_ptr->getDevice(), vertexStaging->memory, 0, vertexSize, 0, &data);
        memcpy(data, vertices.data(), vertices.size() * sizeof(Rendering::Objects::Vertex));
        vkUnmapMemory(m_deviceContext_ptr->getDevice(), vertexStaging->memory);

        vkMapMemory(m_deviceContext_ptr->getDevice(), indexStaging->memory, 0, indexSize, 0, &data);
        memcpy(data, indices.data(), indices.size() * sizeof(uint32_t));
        vkUnmapMemory(m_deviceContext_ptr->getDevice(), indexStaging->memory);

        Allocation alloc = {
            .vertexOffset = m_vertexBuffer_ptr->currentOffset,
            .indexOffset = m_indexBuffer_ptr->currentOffset,
            .vertexSize = vertexSize,
            .indexSize = indexSize,
            .vertexStaging = *vertexStaging,
            .indexStaging = *indexStaging,
            // .dynamicUBOIndex = nextUBOIndex++
        };
        m_vertexBuffer_ptr->currentOffset += vertexSize;
        m_indexBuffer_ptr->currentOffset += indexSize;

        return alloc;
    }
}