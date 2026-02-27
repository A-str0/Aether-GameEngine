#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/config/paths.h"
#include "core/rendering/render_scene.h"
#include "core/rendering/vulkan/renderer.h"
#include "core/rendering/vulkan/vulkan_context.h"
#include "core/rendering/vulkan/vulkan_device_context.h"
#include "core/rendering/vulkan/vulkan_surface_context.h"
#include "core/rendering/vulkan/vulkan_swapchain_context.h"
#include "core/resource_managment/resource_manager.h"
#include "core/window_managment/window_context.h"

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
    }

    {
        AetherEngine::WindowContext windowContext{"AetherEngine", 800, 600};
        std::vector<const char*> extensions = windowContext.getRequredExtensions();
        AetherEngine::Rendering::VulkanContext vulkanContext{extensions};
        AetherEngine::Rendering::VulkanSurfaceContext surfaceContext{vulkanContext.getInstance(),
                                                                     windowContext.getWindow()};

        auto deviceContext_ptr = std::make_shared<AetherEngine::Rendering::VulkanDeviceContext>(
            vulkanContext.getInstance(), surfaceContext.getSurface());
        auto swapchainContext_ptr = std::make_shared<AetherEngine::Rendering::VulkanSwapchainContext>(
            *deviceContext_ptr, surfaceContext.getSurface(), windowContext.getWindow());
        auto commandManager_ptr = std::make_shared<AetherEngine::Rendering::VulkanCommandManager>(
            deviceContext_ptr, swapchainContext_ptr);
        auto bufferManager_ptr = std::make_shared<AetherEngine::Rendering::VulkanBufferManager>(
            deviceContext_ptr, commandManager_ptr);

        AetherEngine::Rendering::Renderer renderer{*deviceContext_ptr, *swapchainContext_ptr,
                                                   bufferManager_ptr, commandManager_ptr};
        AetherEngine::ResourceManagment::ResourceManager resourceManager{
            *deviceContext_ptr, *swapchainContext_ptr, bufferManager_ptr, commandManager_ptr};

        auto texture = resourceManager.loadTexture(AetherEngine::Config::texturePath("tex.jpg").string());
        auto material = renderer.createMaterial(texture);

        AetherEngine::Rendering::MeshData quadMesh{
            .vertices = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                         {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                         {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                         {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}},
            .indices = {0, 1, 2, 2, 3, 0}};

        AetherEngine::Rendering::RenderScene scene;
        auto model = renderer.createModel({quadMesh}, material);
        scene.addModel(model);

        AetherEngine::UniformBufferObject ubo{};
        ubo.model = glm::mat4(1.0f);
        ubo.view = glm::mat4(1.0f);
        ubo.proj = glm::mat4(1.0f);
        renderer.updateGlobalUniforms(ubo);

        bool running = true;
        SDL_Event event;
        while (running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
            }
            renderer.drawFrame(scene.getMeshes());
        }
    }

    SDL_Quit();
    return 0;
}
