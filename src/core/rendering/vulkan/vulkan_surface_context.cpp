#include <stdexcept>
#include <string>

#include "vulkan_surface_context.h"

namespace AetherEngine::Rendering {
VulkanSurfaceContext::VulkanSurfaceContext(VkInstance instance, SDL_Window* window) : m_instance(instance) {
    if (!window) {
        throw std::runtime_error("SDL_Window is null when creating Vulkan surface");
    }
    if (!SDL_Vulkan_CreateSurface(window, instance, &m_surface)) {
        throw std::runtime_error("Failed to create Vulkan surface: " + std::string(SDL_GetError()));
    }
}

VulkanSurfaceContext::~VulkanSurfaceContext() {
    if (m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}
}  // namespace AetherEngine::Rendering
