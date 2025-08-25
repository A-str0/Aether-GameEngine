#include "window_context.hpp"
#include <iostream>

namespace AetherEngine {
    WindowContext::WindowContext(std::string title, int width, int height) {
        // Create SDL_Window
        m_window = SDL_CreateWindow(
            title.data(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width, height,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
        );

        if (!m_window) {
            throw std::runtime_error("Failed to create SDL_Window: " + std::string(SDL_GetError()));
        }
        std::cout << "SDL_Window successfully created: " << m_window << std::endl;
    }

    WindowContext::~WindowContext() {
        // TODO: m_surface destroying
        if (m_window) {
            SDL_DestroyWindow(m_window);
        }
    }

    std::vector<const char*> WindowContext::getRequredExtensions() {
        uint32_t extensionCount = 0;
        SDL_Vulkan_GetInstanceExtensions(m_window, &extensionCount, nullptr);

        if (extensionCount <= 0) {
            std::cout << "No valid extensions found" << std::endl;
        }

        std::vector<const char*> extensions(extensionCount);
        SDL_Vulkan_GetInstanceExtensions(m_window, &extensionCount, extensions.data());

        return extensions;
    }

    SDL_bool WindowContext::recreateSurface(const AetherEngine::Rendering::VulkanContext& context) {
        if (m_surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(context.getInstance(), m_surface, nullptr);
            m_surface = VK_NULL_HANDLE;
            std::cout << "Old VkSurfaceKHR destroyed" << std::endl;
        }

        // Create new Surface
        SDL_bool result = SDL_Vulkan_CreateSurface(m_window, context.getInstance(), &m_surface);
        if (result) {
            std::cout << "VkSurfaceKHR successfully created: " << m_surface << std::endl;
        } else {
            std::cout << "Failed to create VkSurfaceKHR: " << SDL_GetError() << std::endl;
        }

        return result;
    }

    void WindowContext::handleEvents() {
        // TODO
    }
}