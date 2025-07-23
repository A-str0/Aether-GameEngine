#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#ifndef AETHERENGINE_RENDERING_VULKANCONTEXT_H
#define AETHERENGINE_RENDERING_VULKANCONTEXT_H

namespace AetherEngine::Rendering {
    class VulkanContext {
    public:
        VulkanContext(std::vector<const char*> extensions, bool enableValidationLayers = false);
        virtual ~VulkanContext();

        VkInstance getInstance() const { return m_instance; }

        VulkanContext(const VulkanContext&) = delete;
        VulkanContext& operator=(const VulkanContext&) = delete;

        VulkanContext(VulkanContext&&) noexcept = default;
        VulkanContext& operator=(VulkanContext&&) noexcept = default;
    private:
        VkInstance m_instance = VK_NULL_HANDLE;

        bool m_enableValidationLayers;
    };
}

#endif