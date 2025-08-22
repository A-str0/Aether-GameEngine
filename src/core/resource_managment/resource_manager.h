#ifndef AETHERENGINE_RESOURCES_RESOURCEMANAGER_H
#define AETHERENGINE_RESOURCES_RESOURCEMANAGER_H

#include <string>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

namespace AetherEngine::ResourceManagment {
    class ResourceManager {
    public:
        ResourceManager();
        // ~ResourceManager(); 

        void loadShader(std::string filename);
        u_char* loadTexture(std::string filename);

        void unloadShader();
        void unloadTexture(u_char* pixels);
    private:
        
    };
}

#endif