#include "vulkan_swapchain_context.h"
#include "vulkan_device_context.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace AetherEngine::Rendering {
    VulkanSwapchainContext::VulkanSwapchainContext(VulkanDeviceContext& deviceContext, WindowContext& windowContext) : m_deviceContext(&deviceContext), m_windowContext(&windowContext) {
        createSwapchain();
        createImageViews();
    }

    VulkanSwapchainContext::~VulkanSwapchainContext() {
        cleanup();
    }

    VkSurfaceFormatKHR VulkanSwapchainContext::selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }
        return formats[0];
    }

    VkPresentModeKHR VulkanSwapchainContext::selectPresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
        for (const auto& mode : presentModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return mode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapchainContext::selectExtent(const VkSurfaceCapabilitiesKHR& capabilities, SDL_Window* window) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }
        int width, height;
        SDL_Vulkan_GetDrawableSize(window, &width, &height);
        VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return extent;
    }

    void VulkanSwapchainContext::cleanup() {
        for (auto imageView : m_imageViews) {
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(m_deviceContext->getDevice(), imageView, nullptr);
            }
        }
        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(m_deviceContext->getDevice(), m_swapchain, nullptr);
        }
    }

    void VulkanSwapchainContext::createSwapchain() {
        VkPhysicalDevice physicalDevice = m_deviceContext->getPhysicalDevice();
        VkDevice device = m_deviceContext->getDevice();
        VkSurfaceKHR surface = m_windowContext->getSurface();
        SDL_Window *window = m_windowContext->getWindow();

        // TODO: VkSurfaceCapabilities2KHR capabilities
        VkSurfaceCapabilitiesKHR capabilities;
        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities) != VK_SUCCESS) {
            throw std::runtime_error("Failed to get Surface Capabilities");
        }

        // Get formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if (formatCount == 0) {
            throw std::runtime_error("No valid Surface Formats avaliable");
        }
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

        // Get present modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        if (presentModeCount == 0) {
            throw std::runtime_error("No valid Surface Present Modes avaliable");
        }
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

        VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat(formats);
        VkPresentModeKHR presentMode = selectPresentMode(presentModes);
        m_extent = selectExtent(capabilities, window);
        m_format = surfaceFormat.format;

        // Create swapchain
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = capabilities.minImageCount + 1;
        // createInfo.minImageCount = std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount, capabilities.maxImageCount);
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = m_extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // Set queue families indicies
        QueueFamilyIndices sQueueFamilyIndices = m_deviceContext->getQueueFamilyIndicies();
        uint32_t queueFamilyIndices[] = {sQueueFamilyIndices.graphicsFamily, sQueueFamilyIndices.presentFamily};
        if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain");
        }
    }

    void VulkanSwapchainContext::createImageViews() {
        VkDevice device = m_deviceContext->getDevice();

        // Get swapchain images
        uint32_t imageCount;
        vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr);
        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_images.data());

        // Get image views
        m_imageViews.resize(imageCount);
        for (size_t i = 0; i < imageCount; ++i) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_format;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create ImageView");
            }
        }
    }

    void VulkanSwapchainContext::recreateSwapchain() {
        vkDeviceWaitIdle(m_deviceContext->getDevice());

        cleanup();

        createSwapchain();
        createImageViews();
    }
}