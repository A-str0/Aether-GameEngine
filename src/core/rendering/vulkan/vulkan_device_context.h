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

        VkDevice device() const { return m_device; }
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }
        uint32_t getGraphicsFamily() const { return m_indices.graphicsFamily; }
        uint32_t getPresentFamily() const { return m_indices.presentFamily; }
    private:
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

        VkDevice m_device = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue = VK_NULL_HANDLE;
        VkQueue m_presentQueue = VK_NULL_HANDLE;
        QueueFamilyIndices m_indices;
    };
}