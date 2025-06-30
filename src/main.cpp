#include <iostream>
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "core/gui/sdl/window_context_sdl.h"
#include "core/rendering/vulkan/renderer_vulkan.h"

using namespace AetherEngine::GUI;
using namespace AetherEngine::Rendering;

const char* APPLICATION_NAME = "AetherEngine";

VkInstance vkInstance{nullptr};

SDL_Window* createWindow() {
    SDL_Window* window = SDL_CreateWindow(
        "test", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        500, 
        500,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN
    );

    return window;
}

VkResult initVulkan(unsigned int extensionsCount, std::vector<const char*> extensions) {
    // TODO: Real Architecture
    // Create Renderer 
    // Renderer_Vulkan renderer(&appInfo);
    // uint32_t isRendererCreated = renderer.create();
    // std::cout << "Renderer created: " << isRendererCreated << "\n";

    // Enumerate API versions
    std::vector<uint32_t> versions(1);
    auto res = vkEnumerateInstanceVersion(versions.data());
    
    uint32_t mainVersion = versions[0];

    // Create application info for Vulkan API
    VkApplicationInfo appInfo{};
    // appInfo.pNext = VkValidationFlagsEXT // i may wish disable validation in future
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = APPLICATION_NAME;
    appInfo.applicationVersion = mainVersion;
    appInfo.apiVersion = mainVersion;
    appInfo.pEngineName = "AetherEngine"; // Or AetherEngine???
    appInfo.engineVersion = mainVersion;

    // VkInstance create info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Enable extensions
    createInfo.enabledExtensionCount = extensionsCount;
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance);

    return result;
}

int main() {
    // Dynamically load Vulkan
    SDL_Vulkan_LoadLibrary(NULL);

    // TODO: Real Architecture
    // Create Window Context
    // WindowContext_SDL windowContext(SDL_INIT_VIDEO);
    // uint32_t isWindowCreated = windowContext.create(
    //     "test", 
    //     SDL_WINDOWPOS_UNDEFINED, 
    //     SDL_WINDOWPOS_UNDEFINED,
    //     500,
    //     500,
    //     SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN
    // );
    // std::cout << "Window created: " << isWindowCreated << "\n";
    SDL_Window* mainWindow = createWindow();    

    // Get extensions
    unsigned int extensionsCount;
    SDL_Vulkan_GetInstanceExtensions(mainWindow, &extensionsCount, nullptr); // Get extensions count
    std::vector<const char*> extensions(extensionsCount);

    SDL_bool extensionsReceived = SDL_Vulkan_GetInstanceExtensions(mainWindow, &extensionsCount, extensions.data());

    if (extensionsReceived) {
        std::cout << "Extensions received succsesfully" << "\n";
    } else {
        std::cout << "Extensions receive failed :(" << "\n";
    }

    // Initializing Vulkan
    VkResult vkInstanceResult = initVulkan(extensionsCount, extensions);
    std::cout << "Vulkan instance initialize result: " << vkInstanceResult << "\n";

    // Get PhysicalDevices
    uint32_t physicalDevicesCount;
    vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, nullptr); // Get physical devices count
    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);

    VkResult vkPhysicalDevicesResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, physicalDevices.data());

    std::cout << "Get Physical Devices result: " << vkPhysicalDevicesResult << "\n";
    for (auto device : physicalDevices) {
        std::cout << "Physical Device: " << device << "\n";
    }

    // Get PhysicalDevices Groups
    uint32_t physicalDevicesGroupsCount;
    vkEnumeratePhysicalDeviceGroups(vkInstance, &physicalDevicesGroupsCount, nullptr);
    std::vector<VkPhysicalDeviceGroupProperties> physicalDevicesGroupsProperties(physicalDevicesGroupsCount);
    VkResult vkPhysicalDevicesGroupsResult = vkEnumeratePhysicalDeviceGroups(
        vkInstance, 
        &physicalDevicesGroupsCount, 
        physicalDevicesGroupsProperties.data()
    );
    // To support multi-GPU systems we should use
    // vkEnumeratePhysicalDeviceGroupsKHR and enable extension
    // VK_KHR_device_group_creation

    std::cout << "Get Physical Devices Groups result: " << vkPhysicalDevicesGroupsResult << "\n";
    for (auto group : physicalDevicesGroupsProperties) {
        std::cout << "Physical Device Group: " << group.sType << "\n";
    }

    // Create Logical Devices
    std::vector<VkDevice> devices(physicalDevicesCount); 

    std::cout << "Creating logical devices";
    for (auto physicalDevice : physicalDevices) {
        VkDevice device{nullptr};
        VkPhysicalDeviceProperties2 deviceProperties;

        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        // deviceCreateInfo.queueCreateInfoCount = // TODO
        // deviceCreateInfo.pQueueCreateInfos = // TODO

        VkResult vkCreateDeviceResult = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
        std::cout << "Create logical device from []" << physicalDevice << "] device result: " << vkCreateDeviceResult << "\n";

        devices.push_back(device);
    }

    // I can destroy devices
    // vkDestroyDevice(device, nullptr);

    return 0;
}