#include <iostream>
#include "vulkan_context.h"

namespace AetherEngine::Rendering {
    VulkanContext::VulkanContext(std::vector<const char*> extensions, bool enableValidationLayers) : m_enableValidationLayers(enableValidationLayers) {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "AetherEngine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "AetherEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        const char* validationLayer = "VK_LAYER_KHRONOS_validation";
        if (enableValidationLayers) {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            bool layerFound = false;
            for (const auto& layer : availableLayers) {
                if (std::strcmp(layer.layerName, validationLayer) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                throw std::runtime_error("Validation Layers not supported");
            }
            createInfo.enabledLayerCount = 1;
            createInfo.ppEnabledLayerNames = &validationLayer;
        } else {
            createInfo.enabledLayerCount = 0;
        }

        // TODO: make debug messenger
        // VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        // if (enableValidationLayers) {
        //     debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        //     debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        //                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        //                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        //     debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        //                                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        //                                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        //     // debugCreateInfo.pfnUserCallback = debugCallback;
        //     createInfo.pNext = &debugCreateInfo;
        // }

        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan Instance");
        }
    }

    VulkanContext::~VulkanContext() {
        if (m_instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_instance, nullptr);
        }
    }

    // std::vector<const char*> VulkanContext::getRequiredExtensions(SDL_Window* window, bool enableValidationLayers) {
    //     uint32_t extensionCount = 0;
    //     SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    //     std::vector<const char*> extensions(extensionCount);
    //     SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data());

    //     if (enableValidationLayers) {
    //         extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    //     }
    //     return extensions;
    // }
}
