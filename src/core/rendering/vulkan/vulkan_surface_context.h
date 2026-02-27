#ifndef AETHERENGINE_RENDERING_VULKAN_SURFACE_CONTEXT_H
#define AETHERENGINE_RENDERING_VULKAN_SURFACE_CONTEXT_H

#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace AetherEngine::Rendering {
    class VulkanSurfaceContext {
    public:
        VulkanSurfaceContext(VkInstance instance, SDL_Window* window);
        ~VulkanSurfaceContext();

        VulkanSurfaceContext(const VulkanSurfaceContext&) = delete;
        VulkanSurfaceContext& operator=(const VulkanSurfaceContext&) = delete;

        VulkanSurfaceContext(VulkanSurfaceContext&&) = delete;
        VulkanSurfaceContext& operator=(VulkanSurfaceContext&&) = delete;

        VkSurfaceKHR getSurface() const { return m_surface; }
    private:
        VkInstance m_instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    };
}

#endif
