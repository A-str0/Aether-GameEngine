#include <cstring>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "vulkan_device_context.h"

namespace AetherEngine::Rendering {
VulkanDeviceContext::VulkanDeviceContext(VkInstance instance, VkSurfaceKHR surface) {
    m_physicalDevice = pickPhysicalDevice(instance, surface);
    m_indices = findQueueFamilies(m_physicalDevice, surface);
    if (m_indices.graphicsFamily == UINT32_MAX || m_indices.presentFamily == UINT32_MAX) {
        throw std::runtime_error("No queue families found for Graphics or Presentation");
    }

    std::set<uint32_t> uniqueQueueFamilies = {m_indices.graphicsFamily, m_indices.presentFamily};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;  // TODO: make argument???
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
    vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount,
                                         availableExtensions.data());

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

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    // Get queues
    vkGetDeviceQueue(m_device, m_indices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_indices.presentFamily, 0, &m_presentQueue);
    vkGetDeviceQueue(m_device, m_indices.transferFamily, 0, &m_transferQueue);
}

VulkanDeviceContext::~VulkanDeviceContext() {
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
    }
}

VkPhysicalDevice VulkanDeviceContext::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No physical devices with Vulkan support found");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (auto device : devices) {
        QueueFamilyIndices indices;
        if (isDeviceSuitable(device, surface, indices)) {
            return device;
        }
    }

    throw std::runtime_error("Failed to find a suitable Vulkan physical device");
}

bool VulkanDeviceContext::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
                                           QueueFamilyIndices& outIndices) {
    outIndices = findQueueFamilies(device, surface);
    if (outIndices.graphicsFamily == UINT32_MAX || outIndices.presentFamily == UINT32_MAX) {
        return false;
    }

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    bool swapchainSupported = false;
    for (const auto& ext : availableExtensions) {
        if (std::strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
            swapchainSupported = true;
            break;
        }
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return swapchainSupported && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices VulkanDeviceContext::findQueueFamilies(VkPhysicalDevice physicalDevice,
                                                          VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    // TODO: review this code
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        // Graphics Family
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        // TODO: Compute Family
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
        }
        // Transfer Family
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.transferFamily = i;
        }

        // Present Family
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        // Quit the cycle
        if (indices.graphicsFamily != UINT32_MAX && indices.presentFamily != UINT32_MAX) {
            break;
        }
    }

    if (indices.transferFamily == UINT32_MAX) {
        indices.transferFamily = indices.graphicsFamily;
    }

    return indices;
}

uint32_t VulkanDeviceContext::getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties2 memoryProperties{};
    memoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    vkGetPhysicalDeviceMemoryProperties2(m_physicalDevice, &memoryProperties);

    // TODO: make better way for defining
    for (uint32_t i = 0; i < memoryProperties.memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memoryProperties.memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}
}  // namespace AetherEngine::Rendering
