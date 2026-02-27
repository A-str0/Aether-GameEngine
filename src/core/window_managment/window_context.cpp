#include "window_context.h"
#include <iostream>
#include <stdexcept>

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

    void WindowContext::handleEvents() {
        // TODO
    }
}
