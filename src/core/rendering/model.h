#ifndef AETHERENGINE_RENDERING_MODEL_H
#define AETHERENGINE_RENDERING_MODEL_H

#include <vector>

#include "core/rendering/vulkan/mesh_component.hpp"

namespace AetherEngine::Rendering {
class Model {
   public:
    void addMesh(const MeshComponent& mesh) { m_meshes.push_back(mesh); }

    const std::vector<MeshComponent>& getMeshes() const { return m_meshes; }

   private:
    std::vector<MeshComponent> m_meshes;
};
}  // namespace AetherEngine::Rendering

#endif
