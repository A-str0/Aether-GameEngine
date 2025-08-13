#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include "vulkan_device_context.h"

#ifndef AETHERENGINE_RENDERING_SWAPCHAINCONTEXT_H
#define AETHERENGINE_RENDERING_SWAPCHAINCONTEXT_H

namespace AetherEngine::Rendering {
    class VulkanSwapchainContext {
    public:
        VulkanSwapchainContext(AetherEngine::Rendering::VulkanDeviceContext& deviceContext, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SDL_Window* window);
        ~VulkanSwapchainContext();

        VulkanSwapchainContext(const VulkanSwapchainContext&) = delete;
        VulkanSwapchainContext& operator=(const VulkanSwapchainContext&) = delete;

        VulkanSwapchainContext(VulkanSwapchainContext&&) noexcept = default;
        VulkanSwapchainContext& operator=(VulkanSwapchainContext&&) noexcept = default;

        VkSwapchainKHR getSwapchain() const { return m_swapchain; }
        const std::vector<VkImageView>& getImageViews() const { return m_imageViews; }
        VkFormat getFormat() const { return m_format; }
        VkExtent2D getExtent() const { return m_extent; }

        void recreateSwapchain();
    private:
        VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        VkPresentModeKHR selectPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
        VkExtent2D selectExtent(const VkSurfaceCapabilitiesKHR& capabilities, SDL_Window* window);

        VkDevice m_device;
        VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> m_images; // TODO: change data structures
        std::vector<VkImageView> m_imageViews;
        VkFormat m_format;
        VkExtent2D m_extent;
    };
}

#endif