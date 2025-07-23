#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#ifndef AETHERENGINE_RENDERING_SWAPCHAINCONTEXT_H
#define AETHERENGINE_RENDERING_SWAPCHAINCONTEXT_H

namespace AetherEngine::Rendering {
    class VulkanSwapchainContext {
    public:
        VulkanSwapchainContext(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SDL_Window* window);
        ~VulkanSwapchainContext();

        VkSwapchainKHR swapchain() const { return _swapchain; }
        const std::vector<VkImageView>& imageViews() const { return _imageViews; }
        VkFormat format() const { return _format; }
        VkExtent2D extent() const { return _extent; }

        VulkanSwapchainContext(const VulkanSwapchainContext&) = delete;
        VulkanSwapchainContext& operator=(const VulkanSwapchainContext&) = delete;

        VulkanSwapchainContext(VulkanSwapchainContext&&) noexcept = default;
        VulkanSwapchainContext& operator=(VulkanSwapchainContext&&) noexcept = default;

    private:
        VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        VkPresentModeKHR selectPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
        VkExtent2D selectExtent(const VkSurfaceCapabilitiesKHR& capabilities, SDL_Window* window);

        VkDevice _device;
        VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> _images;
        std::vector<VkImageView> _imageViews;
        VkFormat _format;
        VkExtent2D _extent;
    };
}

#endif