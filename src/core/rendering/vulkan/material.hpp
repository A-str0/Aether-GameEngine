#ifndef AETHERENGINE_RENDERING_MATERIAL_H
#define AETHERENGINE_RENDERING_MATERIAL_H

#include <memory>

#include <vulkan/vulkan.hpp>

#include <core/resource_managment/objects/texture_resource.h>

namespace AetherEngine::Rendering {
    class Material {
    public:
        Material(VkDescriptorSet descriptorSet, std::shared_ptr<ResourceManagment::Objects::TextureResource> textureResource_ptr);
        ~Material();

        VkDescriptorSet getDescriptorSet() const { return m_descriptorSet; }
        const std::shared_ptr<ResourceManagment::Objects::TextureResource>& getTexture() const { return m_textureResource_ptr; }
    private:
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

        std::shared_ptr<ResourceManagment::Objects::TextureResource> m_textureResource_ptr;
    };
}

#endif
