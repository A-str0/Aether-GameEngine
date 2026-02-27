#include "material.hpp"

namespace AetherEngine::Rendering {
    Material::Material(
        VkDescriptorSet descriptorSet, 
        std::shared_ptr<ResourceManagment::Objects::TextureResource> textureResource_ptr
    ) 
        : m_descriptorSet(descriptorSet),
        m_textureResource_ptr(textureResource_ptr)
    {
        
    }

    Material::~Material() = default;

    // void Material::setTexture(uint32_t binding, ResourceManagment::Objects::TextureResource* textureResource, VkSampler* sampler) {
    //     m_textureResource_ptr = textureResource;
        
    //     VkDescriptorImageInfo imageInfo{};
    //     imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //     imageInfo.imageView = textureResource->imageView;
    //     imageInfo.sampler = *sampler;

    //     VkWriteDescriptorSet descriptorWrite{};
    //     descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //     descriptorWrite.dstSet = m_descriptorSet;
    //     descriptorWrite.dstBinding = binding;
    //     descriptorWrite.dstArrayElement = 0;
    //     descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //     descriptorWrite.descriptorCount = 1;
    //     descriptorWrite.pImageInfo = &imageInfo;

    //     vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
    // }   
}
