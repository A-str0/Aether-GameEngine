#include "vulkan_device_context.h"
#include <set>
#include <stdexcept>
#include <string>

namespace AetherEngine::Rendering {
    VulkanDeviceContext::VulkanDeviceContext(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        m_indices = findQueueFamilies(physicalDevice, surface);
        if (m_indices.graphicsFamily == UINT32_MAX || m_indices.presentFamily == UINT32_MAX) {
            throw std::runtime_error("No queue families found for Graphics or Presentation");
        }

        std::set<uint32_t> uniqueQueueFamilies = {m_indices.graphicsFamily, m_indices.presentFamily};
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f; // TODO: make argument???
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Check if device supports swapchain
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        bool swapchainSupported = false;
        for (const auto& ext : availableExtensions) {
            if (std::strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                swapchainSupported = true;
                break;
            }
        }
        if (!swapchainSupported) {
            throw std::runtime_error("The device does not support the VK_KHR_swapchain extension");
        }

        // Create logical device
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

        const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device");
        }

        // Get queues
        vkGetDeviceQueue(m_device, m_indices.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_indices.presentFamily, 0, &m_presentQueue);
    }

    VulkanDeviceContext::~VulkanDeviceContext() {
        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, nullptr);
        }
    }

    QueueFamilyIndices VulkanDeviceContext::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }
            if (indices.graphicsFamily != UINT32_MAX && indices.presentFamily != UINT32_MAX) {
                break;
            }
        }

        return indices;
    }
}