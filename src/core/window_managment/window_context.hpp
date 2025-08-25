#include <SDL2/SDL.h>
#include <rendering/vulkan/vulkan_context.hpp>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#ifndef AETHERENGINE_WINDOWCONTEXT_H 
#define AETHERENGINE_WINDOWCONTEXT_H

namespace AetherEngine {
    class WindowContext {
    public:
        WindowContext(std::string title, int width, int height);
        ~WindowContext();

        WindowContext(const WindowContext&) = delete;
        WindowContext& operator=(const WindowContext&) = delete;

        WindowContext(WindowContext&&) = default;
        WindowContext& operator=(WindowContext&&) = default;

        VkSurfaceKHR getSurface() { return m_surface; } 
        SDL_bool recreateSurface(const AetherEngine::Rendering::VulkanContext& context);

        SDL_Window* getWindow() { return m_window; } 

        std::vector<const char*> getRequredExtensions();

        void handleEvents();

        // SDL_Window* getWindow() { return m_window; }
    private:
        SDL_Window* m_window = nullptr;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    };
}

#endif