#ifndef AETHERENGINE_CONFIG_PATHS_H
#define AETHERENGINE_CONFIG_PATHS_H

#include <filesystem>
#include <string>

#ifndef AETHER_ENGINE_ASSETS_DIR
#define AETHER_ENGINE_ASSETS_DIR "."
#endif

namespace AetherEngine::Config {
inline std::filesystem::path assetRoot() { return std::filesystem::path(AETHER_ENGINE_ASSETS_DIR); }

inline std::filesystem::path shaderPath(const std::string& filename) {
    return assetRoot() / "compiled_shaders" / filename;
}

inline std::filesystem::path texturePath(const std::string& filename) {
    return assetRoot() / "textures" / filename;
}
}  // namespace AetherEngine::Config

#endif
