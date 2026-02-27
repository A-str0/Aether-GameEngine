#ifndef AETHERENGINE_RENDERING_RENDER_SCENE_H
#define AETHERENGINE_RENDERING_RENDER_SCENE_H

#include <vector>

#include "core/rendering/model.h"
#include "core/rendering/vulkan/mesh_component.hpp"

namespace AetherEngine::Rendering {
class RenderScene {
   public:
    void addMesh(const MeshComponent& mesh) { m_meshes.push_back(mesh); }

    void addMeshes(const std::vector<MeshComponent>& meshes) {
        m_meshes.insert(m_meshes.end(), meshes.begin(), meshes.end());
    }

    void addModel(const Model& model) { addMeshes(model.getMeshes()); }

    void clear() { m_meshes.clear(); }

    const std::vector<MeshComponent>& getMeshes() const { return m_meshes; }

   private:
    std::vector<MeshComponent> m_meshes;
};
}  // namespace AetherEngine::Rendering

#endif
