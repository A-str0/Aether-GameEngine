#include <window_managment/window_context.hpp>
#include <rendering/vulkan/vulkan_context.hpp>
#include <rendering/vulkan/vulkan_device_context.hpp>
#include <rendering/vulkan/vulkan_swapchain_context.hpp>
#include <rendering/vulkan/renderer.hpp>
#include <resource_managment/resource_manager.hpp>

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

    // TODO: refactor
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkanContext.getInstance(), &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("No physical devices with Vulkan support found");
    }
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanContext.getInstance(), &deviceCount, physicalDevices.data());

    VkPhysicalDevice physicalDevice{};
    for (uint32_t i = 0; i < deviceCount; ++i) {
        physicalDevice = physicalDevices[i];

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

        if (supportedFeatures.samplerAnisotropy) {
            break;
        }
    }

    AetherEngine::Rendering::VulkanDeviceContext deviceContext {physicalDevice, windowContext.getSurface()};
    AetherEngine::Rendering::VulkanSwapchainContext swapchainContext {deviceContext, windowContext};
    AetherEngine::Rendering::Renderer renderer {deviceContext, swapchainContext, windowContext.getSurface()};
    AetherEngine::ResourceManagment::ResourceManager resourceManager {deviceContext, swapchainContext, renderer};

    auto texture = resourceManager.loadTexture("../../../src/core/rendering/textures/tex.jpg");
    renderer.updateDescriptorSets(texture->imageView);

    // Create a vector to hold models
    std::vector<std::shared_ptr<AetherEngine::Rendering::Objects::Model>> models;

    // Create quad model using ResourceManager
    auto quadModel = resourceManager.createQuadModel();
    models.push_back(quadModel);

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
        renderer.drawFrame(models);
    }

    // TODO: change?
    texture->cleanup(deviceContext.getDevice());

    SDL_Quit();
    return 0;
}