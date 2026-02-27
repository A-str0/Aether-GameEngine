# Aether-GameEngine

## Rendering Quickstart

High-level flow (see `src/main.cpp` for a working example):

1. Create a `TextureResource` via `ResourceManager::loadTexture`.
2. Create a `Material` via `Renderer::createMaterial`.
3. Build `MeshData` (vertices + indices) and upload via `Renderer::createMesh`.
4. Add meshes to `RenderScene` and call `Renderer::drawFrame(scene.getMeshes())`.
