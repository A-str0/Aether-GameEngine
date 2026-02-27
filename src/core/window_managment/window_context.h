#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <string>

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

        SDL_Window* getWindow() { return m_window; } 

        std::vector<const char*> getRequredExtensions();

        void handleEvents();

        // SDL_Window* getWindow() { return m_window; }
    private:
        SDL_Window* m_window = nullptr;
    };
}

#endif
