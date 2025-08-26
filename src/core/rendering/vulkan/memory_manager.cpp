#include "memory_manager.hpp"

#include <chrono>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>

#include <rendering/objects/vertex.hpp>

namespace AetherEngine::Rendering {
    MemoryManager::MemoryManager(
        VulkanDeviceContext* deviceContext_ptr, 
        VulkanSwapchainContext* swapchainContext_ptr
    ) : 
        m_deviceContext_ptr(deviceContext_ptr),
        m_swapchainContext_ptr(swapchainContext_ptr)
    { 
        createTransferCommandPool();
        createCommandPool();
        createCommandBuffers();

        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffer();

        createDescriptorSetLayout();
        createDescriptorPool();
        createDescriptorSets();
    }

    void MemoryManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void MemoryManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        // if (buffer != VK_NULL_HANDLE) {
        //     vkDestroyBuffer(m_deviceContext_ptr->getDevice(), buffer, nullptr);
        // }
        // if (bufferMemory != VK_NULL_HANDLE) {
        //     vkFreeMemory(m_deviceContext_ptr->getDevice(), bufferMemory, nullptr);
        // }

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.flags = 0; // TODO
        bufferInfo.pNext = nullptr; // TODO
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        
        // Use exclusive sharing mode unless concurrent access is specifically needed
        // For most buffers, exclusive mode is sufficient and avoids queue family index issues
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;
        bufferInfo.pQueueFamilyIndices = nullptr;

        VkResult result = vkCreateBuffer(m_deviceContext_ptr->getDevice(), &bufferInfo, nullptr, &buffer);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer! Error code: " + std::to_string(result));
        }

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(m_deviceContext_ptr->getDevice(), buffer, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = m_deviceContext_ptr->getMemoryType(memoryRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(m_deviceContext_ptr->getDevice(), &allocInfo, nullptr, &bufferMemory);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory! Error code: " + std::to_string(result));
        }

        result = vkBindBufferMemory(m_deviceContext_ptr->getDevice(), buffer, bufferMemory, 0);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to bind buffer memory! Error code: " + std::to_string(result));
        }
    }

    void MemoryManager::createVertexBuffer() {
        // Create vertex buffer
        VkDeviceSize vertexBufferSize = sizeof(Rendering::Objects::Vertex) * 4; // Default size for 4 vertices
        
        createBuffer(
            vertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_vertexBuffer,
            m_vertexBufferMemory
        );
    }

    void MemoryManager::createIndexBuffer() {
        // Create index buffer
        VkDeviceSize indexBufferSize = sizeof(uint16_t) * 6; // Default size for 6 indices
        
        createBuffer(
            indexBufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_indexBuffer,
            m_indexBufferMemory
        );
    }

    void MemoryManager::createUniformBuffer() {
        VkDeviceSize bufferSize = sizeof(AetherEngine::UniformBufferObject);

        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(
                bufferSize, 
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                m_uniformBuffers[i], 
                m_uniformBuffersMemory[i]
            );

            vkMapMemory(m_deviceContext_ptr->getDevice(), m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
        }
    }
    
    void MemoryManager::createCommandPool() {
        // Create Command Pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_deviceContext_ptr->getGraphicsFamily();

        if (vkCreateCommandPool(m_deviceContext_ptr->getDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void MemoryManager::createTransferCommandPool() {
        // Create Transfer Command Pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_deviceContext_ptr->getTransferFamily();

        if (vkCreateCommandPool(m_deviceContext_ptr->getDevice(), &poolInfo, nullptr, &m_transferCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create transfer command pool!");
        }
    }
    
    void MemoryManager::createCommandBuffers() {
        // Allocate Command Buffer
        auto imageCount = m_swapchainContext_ptr->getImageViews().size();
        m_commandBuffers.resize(imageCount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_deviceContext_ptr->getDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    VkCommandBuffer MemoryManager::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_transferCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        // TODO: make cooler memory allocation (don't call vkAllocateCommandBuffers for every buffer)
        vkAllocateCommandBuffers(m_deviceContext_ptr->getDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void MemoryManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // TODO: change Queue to submit
        vkQueueSubmit(m_deviceContext_ptr->getTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        // TODO: fence usage
        vkQueueWaitIdle(m_deviceContext_ptr->getGraphicsQueue());

        vkFreeCommandBuffers(m_deviceContext_ptr->getDevice(), m_transferCommandPool, 1, &commandBuffer);
    }

    void MemoryManager::createDescriptorSetLayout() {
        std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
        
        // Uniform buffer binding (binding 0)
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        bindings[0].pImmutableSamplers = nullptr;

        // Combined image sampler binding (binding 1)
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_deviceContext_ptr->getDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }


    void MemoryManager::createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(m_deviceContext_ptr->getDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }

    void MemoryManager::createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(layouts.size());
        if (vkAllocateDescriptorSets(m_deviceContext_ptr->getDevice(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }
    }

    void MemoryManager::uploadMesh(const AetherEngine::Rendering::Objects::Mesh& mesh) {
        // Check if we need to recreate vertex buffer (if current buffer is too small or doesn't exist)
        VkDeviceSize requiredVertexBufferSize = sizeof(Rendering::Objects::Vertex) * mesh.vertices.size();
        
        if (m_vertexBuffer == VK_NULL_HANDLE) {
            // Create new buffer with proper size
            createBuffer(
                requiredVertexBufferSize,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_vertexBuffer,
                m_vertexBufferMemory
            );
        } else {
            // Check if existing buffer is too small
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(m_deviceContext_ptr->getDevice(), m_vertexBuffer, &memRequirements);
            
            if (memRequirements.size < requiredVertexBufferSize) {
                // Destroy old buffer and create new one with proper size
                vkDestroyBuffer(m_deviceContext_ptr->getDevice(), m_vertexBuffer, nullptr);
                vkFreeMemory(m_deviceContext_ptr->getDevice(), m_vertexBufferMemory, nullptr);
                
                createBuffer(
                    requiredVertexBufferSize,
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_vertexBuffer,
                    m_vertexBufferMemory
                );
            }
        }

        // Check if we need to recreate index buffer (if current buffer is too small or doesn't exist)
        VkDeviceSize requiredIndexBufferSize = sizeof(uint16_t) * mesh.indices.size();
        
        if (m_indexBuffer == VK_NULL_HANDLE) {
            // Create new buffer with proper size
            createBuffer(
                requiredIndexBufferSize,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m_indexBuffer,
                m_indexBufferMemory
            );
        } else {
            // Check if existing buffer is too small
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(m_deviceContext_ptr->getDevice(), m_indexBuffer, &memRequirements);
            
            if (memRequirements.size < requiredIndexBufferSize) {
                // Destroy old buffer and create new one with proper size
                vkDestroyBuffer(m_deviceContext_ptr->getDevice(), m_indexBuffer, nullptr);
                vkFreeMemory(m_deviceContext_ptr->getDevice(), m_indexBufferMemory, nullptr);
                
                createBuffer(
                    requiredIndexBufferSize,
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    m_indexBuffer,
                    m_indexBufferMemory
                );
            }
        }

        // Copy vertex data to vertex buffer
        VkDeviceSize vertexBufferSize = sizeof(Rendering::Objects::Vertex) * mesh.vertices.size();
        
        // Create staging buffer for vertices
        VkBuffer vertexStagingBuffer;
        VkDeviceMemory vertexStagingBufferMemory;
        createBuffer(
            vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexStagingBuffer,
            vertexStagingBufferMemory
        );

        // Map memory and copy vertex data
        void* vertexData;
        vkMapMemory(m_deviceContext_ptr->getDevice(), vertexStagingBufferMemory, 0, vertexBufferSize, 0, &vertexData);
        memcpy(vertexData, mesh.vertices.data(), static_cast<size_t>(vertexBufferSize));
        vkUnmapMemory(m_deviceContext_ptr->getDevice(), vertexStagingBufferMemory);

        // Copy staging buffer to device local vertex buffer
        copyBuffer(vertexStagingBuffer, m_vertexBuffer, vertexBufferSize);

        // Cleanup staging buffer
        vkDestroyBuffer(m_deviceContext_ptr->getDevice(), vertexStagingBuffer, nullptr);
        vkFreeMemory(m_deviceContext_ptr->getDevice(), vertexStagingBufferMemory, nullptr);

        // Copy index data to index buffer
        VkDeviceSize indexBufferSize = sizeof(uint16_t) * mesh.indices.size();
        
        // Create staging buffer for indices
        VkBuffer indexStagingBuffer;
        VkDeviceMemory indexStagingBufferMemory;
        createBuffer(
            indexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            indexStagingBuffer,
            indexStagingBufferMemory
        );

        // Map memory and copy index data
        void* indexData;
        vkMapMemory(m_deviceContext_ptr->getDevice(), indexStagingBufferMemory, 0, indexBufferSize, 0, &indexData);
        memcpy(indexData, mesh.indices.data(), static_cast<size_t>(indexBufferSize));
        vkUnmapMemory(m_deviceContext_ptr->getDevice(), indexStagingBufferMemory);

        // Copy staging buffer to device local index buffer
        copyBuffer(indexStagingBuffer, m_indexBuffer, indexBufferSize);

        // Cleanup staging buffer
        vkDestroyBuffer(m_deviceContext_ptr->getDevice(), indexStagingBuffer, nullptr);
        vkFreeMemory(m_deviceContext_ptr->getDevice(), indexStagingBufferMemory, nullptr);
    }

    void MemoryManager::updateUniformBuffer(uint32_t currentImage) {
        return;

        static auto startTime = std::chrono::high_resolution_clock::now();
    
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    
        // TODO: refactor
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 01.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(35.0f), m_swapchainContext_ptr->getExtent().width / (float) m_swapchainContext_ptr->getExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
    
        // Using a UBO this way is not the most efficient way to pass frequently changing values to the shader. 
        // A more efficient way to pass a small buffer of data to shaders are push constants
        memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }
}