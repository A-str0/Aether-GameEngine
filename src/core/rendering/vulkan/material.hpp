#ifndef AETHERENGINE_RENDERING_MATERIAL_H
#define AETHERENGINE_RENDERING_MATERIAL_H

#include <memory>

#include <vulkan/vulkan.hpp>

#include "vulkan_device_context.h"
#include <core/resource_managment/objects/texture_resource.h>

namespace AetherEngine::Rendering {
    class Material {
    public:
        Material(std::weak_ptr<VkDevice> device_ptr, std::shared_ptr<VkDescriptorSet> descriptorSet_ptr, std::shared_ptr<ResourceManagment::Objects::TextureResource> textureResource_ptr);
        ~Material();

        VkDescriptorSet getDescriptorSet() const { return *m_descriptorSet_ptr.get(); }
    private:
        std::weak_ptr<VkDevice> m_device_ptr;
        std::shared_ptr<VkDescriptorSet> m_descriptorSet_ptr;

        std::shared_ptr<ResourceManagment::Objects::TextureResource> m_textureResource_ptr;
    };
}

#endif