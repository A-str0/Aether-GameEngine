#ifndef AETHERENGINE_RENDERING_DEVICECONTEXT_H
#define AETHERENGINE_RENDERING_DEVICECONTEXT_H

#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "structs.h"

namespace AetherEngine::Rendering {
    class VulkanDeviceContext {
    public:
        VulkanDeviceContext(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        ~VulkanDeviceContext();

        VulkanDeviceContext(const VulkanDeviceContext&) = delete;
        VulkanDeviceContext& operator=(const VulkanDeviceContext&) = delete;

        VulkanDeviceContext(VulkanDeviceContext&&) noexcept = default;
        VulkanDeviceContext& operator=(VulkanDeviceContext&&) noexcept = default;

        VkDevice getDevice() const { return m_device; }
        VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }
        VkQueue getTransferQueue() const { return m_transferQueue; }
        uint32_t getGraphicsFamily() const { return m_indices.graphicsFamily; }
        uint32_t getPresentFamily() const { return m_indices.presentFamily; }
        uint32_t getTransferFamily() const { return m_indices.transferFamily; }
        QueueFamilyIndices getQueueFamilyIndicies() const { return m_indices; }

        u_int32_t getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    private:
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

        VkDevice m_device = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        VkQueue m_transferQueue = VK_NULL_HANDLE;
        QueueFamilyIndices m_indices;
    };
}

#endif