#include "core/window_managment/window_context.h"
#include "core/rendering/vulkan/vulkan_context.h"
#include "core/rendering/vulkan/vulkan_device_context.h"
#include "core/rendering/vulkan/vulkan_swapchain_context.h"
#include "core/rendering/vulkan/renderer.h"
#include "core/resource_managment/resource_manager.h"
#include "core/rendering/vulkan/material.hpp"
#include "core/rendering/objects/vertex.h"
#include "core/rendering/vulkan/mesh_component.hpp"
#include "core/rendering/vulkan/material.hpp"
#include "core/rendering/vulkan/vulkan_buffer_manager.hpp"
#include "core/rendering/vulkan/vulkan_command_manager.hpp"

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

    auto deviceContext_ptr = std::make_shared<AetherEngine::Rendering::VulkanDeviceContext>(physicalDevice, windowContext.getSurface());
    auto swapchainContext_ptr = std::make_shared<AetherEngine::Rendering::VulkanSwapchainContext>(*deviceContext_ptr.get(), windowContext);
    auto commandManager_ptr = std::make_shared<AetherEngine::Rendering::VulkanCommandManager>(deviceContext_ptr, swapchainContext_ptr);
    auto bufferManager_ptr = std::make_shared<AetherEngine::Rendering::VulkanBufferManager>(deviceContext_ptr, commandManager_ptr);

    AetherEngine::Rendering::Renderer renderer {*deviceContext_ptr.get(), *swapchainContext_ptr.get(), bufferManager_ptr, commandManager_ptr};
    AetherEngine::ResourceManagment::ResourceManager resourceManager {*deviceContext_ptr.get(), *swapchainContext_ptr.get(), bufferManager_ptr, commandManager_ptr};

    auto texture = resourceManager.loadTexture("../../../src/core/rendering/textures/tex.jpg");
    auto material = std::make_shared<AetherEngine::Rendering::Material>(std::make_shared<VkDevice>(deviceContext_ptr->getDevice()), nullptr, texture);

    const std::vector<AetherEngine::Rendering::Objects::Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };
    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };
    auto mesh = AetherEngine::Rendering::MeshComponent {vertices, indices, 0, 0, material};

    std::vector<AetherEngine::Rendering::MeshComponent> meshes;
    meshes.push_back(mesh);

    // AetherEngine::Rendering::Material material{texture, renderer.};
    
    // renderer.updateDescriptorSets(texture->imageView);

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
        renderer.drawFrame(meshes);
    }

    // TODO: change?
    // texture->cleanup(deviceContext.getDevice());

    SDL_Quit();
    return 0;
}