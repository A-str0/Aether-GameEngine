#include <fstream>

#include "resource_manager.h"
#include <stb/stb_image.h>

namespace AetherEngine::ResourceManagment {
    ResourceManager::ResourceManager() {
        
    }

    u_char* ResourceManager::loadTexture(std::string filename) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filename.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }

        return pixels;
    }

    void ResourceManager::unloadTexture(stbi_uc* pixels) {
        if (!pixels) {
            throw std::runtime_error("Pixels array is null!");
        }

        stbi_image_free(pixels);
    }
}