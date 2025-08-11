#include "core/window_context.h"
#include "core/rendering/vulkan/vulkan_context.h"
#include "core/rendering/vulkan/vulkan_device_context.h"
#include "core/rendering/vulkan/vulkan_swapchain_context.h"
#include "core/rendering/vulkan/renderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <stdexcept>
#include <vector>
#include <string>


int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
    }

    AetherEngine::WindowContext windowContext {"AetherEngine", 800, 600};
    std::vector<const char*> extensions = windowContext.getRequredExtensions();
    AetherEngine::Rendering::VulkanContext vulkanContext {extensions};
    windowContext.recreateSurface(vulkanContext);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkanContext.getInstance(), &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No physical devices with Vulkan support found");
    }
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanContext.getInstance(), &deviceCount, physicalDevices.data());
    VkPhysicalDevice physicalDevice = physicalDevices[0]; // TODO: cooler selection
    
    AetherEngine::Rendering::VulkanDeviceContext deviceContext {physicalDevice, windowContext.getSurface()};
    AetherEngine::Rendering::VulkanSwapchainContext swapchainContext {deviceContext, physicalDevice, windowContext.getSurface(), windowContext.getWindow()};
    AetherEngine::Rendering::Renderer renderer {deviceContext, swapchainContext, windowContext.getSurface()};

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } 
            // else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            //     recreateSwapchain();
            // }
        }
        renderer.drawFrame();
    }

    SDL_Quit();
    return 0;
}