#ifndef AETHERENGINE_RESOURCES_RESOURCEMANAGER_H
#define AETHERENGINE_RESOURCES_RESOURCEMANAGER_H

#include <string>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

namespace AetherEngine::Resources {
    class ResourceManager {
    public:
        ResourceManager();
        // ~ResourceManager(); 

        u_char* loadTexture(std::string filename);
        void unloadTexture(u_char* pixels);
    };
}

#endif