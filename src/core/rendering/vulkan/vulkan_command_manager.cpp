#include "vulkan_command_manager.hpp"

namespace AetherEngine::Rendering {
    VulkanCommandManager::VulkanCommandManager(
        std::shared_ptr<VulkanDeviceContext> deviceContext_ptr, 
        std::shared_ptr<VulkanSwapchainContext> swapchainContext_ptr
    ) :
        m_deviceContext_ptr(deviceContext_ptr),
        m_swapchainContext_ptr(swapchainContext_ptr)
    {

    }

    VulkanCommandManager::~VulkanCommandManager() {
        VkDevice device = m_deviceContext_ptr->getDevice();

        for (auto semaphore : m_imageAvailableSemaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
        for (auto semaphore : m_renderFinishedSemaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
        for (auto fence : m_inFlightFences) {
            vkDestroyFence(device, fence, nullptr);
        }

        vkDestroyCommandPool(device, m_commandPool, nullptr);
    }

    void VulkanCommandManager::createTransferCommandPool() {
        // Create Transfer Command Pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_deviceContext_ptr->getTransferFamily();

        if (vkCreateCommandPool(m_deviceContext_ptr->getDevice(), &poolInfo, nullptr, &m_transferCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create transfer command pool!");
        }
    }

    void VulkanCommandManager::createCommandPool() {
        // Create Command Pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_deviceContext_ptr->getGraphicsFamily();

        if (vkCreateCommandPool(m_deviceContext_ptr->getDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void VulkanCommandManager::createCommandBuffers() {
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

    VkCommandBuffer VulkanCommandManager::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
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

    void VulkanCommandManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // TODO: change Queue to submit
        vkQueueSubmit(m_deviceContext_ptr->getTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        // TODO: fence usage
        vkQueueWaitIdle(m_deviceContext_ptr->getGraphicsQueue());

        vkFreeCommandBuffers(m_deviceContext_ptr->getDevice(), m_commandPool, 1, &commandBuffer);
    }

    void VulkanCommandManager::createSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(m_deviceContext_ptr->getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_deviceContext_ptr->getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_deviceContext_ptr->getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create synchronization objects!");
            }
        }
    }
}