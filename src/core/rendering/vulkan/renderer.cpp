#define GLM_FORCE_RADIANS

#include "renderer.h"

#include <stdexcept>
#include <array>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "../objects/vertex.h" // TODO: change
#include "../../resource_managment/objects/texture_resource.h" // TODO: change
#include <core/rendering/vulkan/uniform_buffer_object.hpp>

namespace AetherEngine::Rendering {
    std::vector<char> readShaderFile(const std::string& filename) {
        std::filesystem::path path = std::filesystem::absolute(filename);

        std::cout << "CWD: " << std::filesystem::current_path().string() << "\n";
        std::cout << "Trying to open: " << path.string() << "\n";

        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Shader file not found: " + path.string());
        }
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + path.string());
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        if (fileSize == 0 || fileSize % 4 != 0) {
            file.close();
            throw std::runtime_error("Invalid SPIR-V size: " + path.string());
        }
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        if (!file.good() || file.gcount() != static_cast<std::streamsize>(fileSize)) {
            file.close();
            throw std::runtime_error("Failed to read shader file: " + path.string());
        }
        file.close();
        return buffer;
    }

    Renderer::Renderer(
        VulkanDeviceContext& deviceContext, 
        VulkanSwapchainContext& swapchainContext, 
        std::shared_ptr<VulkanBufferManager> bufferManager_ptr, 
        std::shared_ptr<VulkanCommandManager> commandManager_ptr
    ) : 
        m_deviceContext(deviceContext), 
        m_swapchainContext(swapchainContext), 
        m_bufferManager_ptr(bufferManager_ptr),
        m_commandManager_ptr(commandManager_ptr)
    {
        createRenderPass();
        createShaderModules();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createFramebuffers();

        createDescriptorPool();
        createDescriptorSets();
        
    }

    Renderer::~Renderer() {
        VkDevice device = m_deviceContext.getDevice();
        
        for (auto framebuffer : m_frameBuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
        vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, m_graphicsPipelineLayout, nullptr);
        vkDestroyShaderModule(device, m_vertexShaderModule, nullptr);
        vkDestroyShaderModule(device, m_fragmentShaderModule, nullptr);
        vkDestroyRenderPass(device, m_renderPass, nullptr);
    }

    void Renderer::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapchainContext.getFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.inputAttachmentCount = 0; // Fix
        subpass.pInputAttachments = nullptr;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        vkCreateRenderPass(m_deviceContext.getDevice(), &renderPassInfo, nullptr, &m_renderPass);
    }

    // TODO: use this
    void Renderer::createRenderPass2() {
        // Create attachments
        // Create Color Attachment
        VkAttachmentDescription2 colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
        colorAttachment.pNext = nullptr;
        // colorAttachment.flags = // TODO
        colorAttachment.format = m_swapchainContext.getFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: settings for this
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO: change
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // TODO: change

        // Create Color Attachment Reference
        VkAttachmentReference2 colorAttachmentReference{};
        colorAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
        colorAttachmentReference.pNext = nullptr;
        colorAttachmentReference.attachment = 0; // TODO: change
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentReference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        // Create attachments array
        // TODO

        // Create SubPasses
        VkSubpassDescription2 graphicsSubpass{};
        graphicsSubpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
        // subpass.flags = // TODO
        graphicsSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        graphicsSubpass.inputAttachmentCount = 1; // TODO: change to auto
        graphicsSubpass.pInputAttachments = &colorAttachmentReference; // TODO: change
        graphicsSubpass.colorAttachmentCount = 1; // TODO: change to auto
        graphicsSubpass.pColorAttachments = &colorAttachmentReference; // TODO: change
        graphicsSubpass.pResolveAttachments = nullptr; // TODO
        graphicsSubpass.pDepthStencilAttachment = nullptr; // TODO
        graphicsSubpass.preserveAttachmentCount = 0; // TODO
        graphicsSubpass.pPreserveAttachments = nullptr; // TODO

        // Create RenderPass
        VkRenderPassCreateInfo2 renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
        renderPassCreateInfo.pNext = nullptr;
        renderPassCreateInfo.flags = VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
        renderPassCreateInfo.attachmentCount = 1; // TODO: change to auto
        renderPassCreateInfo.pAttachments = &colorAttachment; // TODO: change
        renderPassCreateInfo.subpassCount = 1; // TODO: change to auto
        renderPassCreateInfo.pSubpasses = &graphicsSubpass; // TODO: change
        // renderPassCreateInfo.dependencyCount = 0; // TODO
        // renderPassCreateInfo.pDependencies = nullptr; // TODO
        vkCreateRenderPass2(m_deviceContext.getDevice(), &renderPassCreateInfo, nullptr, &m_renderPass);
    }

    void Renderer::createShaderModules() {
        // Create ShaderModules
        // Create Vertex ShaderModule
        // TODO: change
        const std::vector<char> vertexCode = readShaderFile("../../../src/core/rendering/compiled_shaders/standard_vertex_shader.spv"); // TODO: normal directory finding

        VkShaderModuleCreateInfo vertexShaderModuleCreateInfo{};
        vertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertexShaderModuleCreateInfo.pNext = nullptr; // TODO
        // vertexShaderModuleCreateInfo.flags = // TODO???
        vertexShaderModuleCreateInfo.codeSize = vertexCode.size();
        vertexShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexCode.data()); // ???

        vkCreateShaderModule(m_deviceContext.getDevice(), &vertexShaderModuleCreateInfo, nullptr, &m_vertexShaderModule);

        // Create Fragment ShaderModule 
        // TODO: change
        const std::vector<char> fragmentCode = readShaderFile("../../../src/core/rendering/compiled_shaders/standard_fragment_shader.spv"); // TODO: normal directory finding

        VkShaderModuleCreateInfo fragmentShaderModuleCreateInfo{};
        fragmentShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragmentShaderModuleCreateInfo.pNext = nullptr; // TODO
        // fragmentShaderModuleCreateInfo.flags = // TODO???
        fragmentShaderModuleCreateInfo.codeSize = fragmentCode.size();
        fragmentShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentCode.data()); // ???

        vkCreateShaderModule(m_deviceContext.getDevice(), &fragmentShaderModuleCreateInfo, nullptr, &m_fragmentShaderModule);

    }

    void Renderer::createGraphicsPipeline() {
        // Create ShaderStages
        // Create Vertex ShaderStage
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = m_vertexShaderModule;
        vertShaderStageInfo.pName = "main";

        // Create Fragment ShaderStage
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = m_fragmentShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        // Create Rendering States
        // Create VertexInput
        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.pNext = nullptr;
        vertexInput.vertexBindingDescriptionCount = 1;

        auto bindingDescription = Objects::Vertex::getBindingDescription();
        auto attributeDescriptions = Objects::Vertex::getAttributeDescriptions();
        vertexInput.pVertexBindingDescriptions = &bindingDescription; // TODO
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();

        // Create InputAssembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Create DynamicStates
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.flags = 0x0;
        dynamicState.pNext = nullptr;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // Create ViewportState
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // Create Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizerState{};
        rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerState.pNext = nullptr; // TODO
        // rasterizerState.flags
        rasterizerState.depthClampEnable = VK_FALSE;
        rasterizerState.rasterizerDiscardEnable = VK_FALSE;
        rasterizerState.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizerState.lineWidth = 1.0f;
        rasterizerState.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizerState.depthBiasEnable = VK_FALSE;
        // rasterizerState.depthBiasConstantFactor = 0.0f; // TODO
        // rasterizerState.depthBiasClamp = 0.0f; // TODO
        // rasterizerState.depthBiasSlopeFactor = 0.0f; // TODO

        // Create Multisampling
        VkPipelineMultisampleStateCreateInfo multisamplingState{};
        multisamplingState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingState.sampleShadingEnable = VK_FALSE;
        multisamplingState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        // multisamplingState.minSampleShading = 1.0f; // TODO
        // multisamplingState.pSampleMask = nullptr; // TODO
        // multisamplingState.alphaToCoverageEnable = VK_FALSE; // TODO
        // multisamplingState.alphaToOneEnable = VK_FALSE; // TODO

        // Create Color Blending
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable = VK_FALSE;
        // colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // TODO
        // colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // TODO
        // colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // TODO
        // colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // TODO
        // colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // TODO
        // colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // TODO

        VkPipelineColorBlendStateCreateInfo colorBlendingState{};
        colorBlendingState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendingState.logicOpEnable = VK_FALSE;
        colorBlendingState.logicOp = VK_LOGIC_OP_COPY;
        colorBlendingState.attachmentCount = 1;
        colorBlendingState.pAttachments = &colorBlendAttachmentState;
        colorBlendingState.blendConstants[0] = 0.0f; // TODO
        colorBlendingState.blendConstants[1] = 0.0f; // TODO
        colorBlendingState.blendConstants[2] = 0.0f; // TODO
        colorBlendingState.blendConstants[3] = 0.0f; // TODO

        // Create Pipeline Layout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; // TODO: auto?
        pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // TODO
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // TODO

        if (vkCreatePipelineLayout(m_deviceContext.getDevice(), &pipelineLayoutInfo, nullptr, &m_graphicsPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        // Create Graphics Pipline
        VkGraphicsPipelineCreateInfo graphicsPiplineInfo{};
        graphicsPiplineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPiplineInfo.stageCount = 2; // TODO: dynamic
        graphicsPiplineInfo.pStages = shaderStages;

        graphicsPiplineInfo.pVertexInputState = &vertexInput;
        graphicsPiplineInfo.pInputAssemblyState = &inputAssembly;
        // graphicsPiplineInfo.flags = nullptr; // TODO
        graphicsPiplineInfo.pTessellationState = nullptr; // TODO
        graphicsPiplineInfo.pViewportState = &viewportState;
        graphicsPiplineInfo.pRasterizationState = &rasterizerState;
        graphicsPiplineInfo.pMultisampleState = &multisamplingState;
        graphicsPiplineInfo.pDepthStencilState = nullptr; // TODO
        graphicsPiplineInfo.pColorBlendState = &colorBlendingState;
        graphicsPiplineInfo.pDynamicState = &dynamicState;
        graphicsPiplineInfo.layout = m_graphicsPipelineLayout;
        graphicsPiplineInfo.renderPass = m_renderPass;
        graphicsPiplineInfo.subpass = 0; // TODO 
        graphicsPiplineInfo.basePipelineHandle = VK_NULL_HANDLE; // TODO
        graphicsPiplineInfo.basePipelineIndex = 0; // TODO

        if (vkCreateGraphicsPipelines(m_deviceContext.getDevice(), VK_NULL_HANDLE, 1, &graphicsPiplineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }
    }

    void Renderer::createFramebuffers() {
        std::vector<VkImageView> imageViews = m_swapchainContext.getImageViews();
        m_frameBuffers.resize(imageViews.size());

        for (uint32_t i = 0; i < imageViews.size(); ++i)
        {
            VkImageView attachments[] = {imageViews[i]};

            // Create Framebuffer
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = nullptr;
            framebufferInfo.flags = 0; // TODO
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.height = m_swapchainContext.getExtent().height;
            framebufferInfo.width = m_swapchainContext.getExtent().width;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_deviceContext.getDevice(), &framebufferInfo, nullptr, &m_frameBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
    }

    void Renderer::createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0; // is it??
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // TODO: learn descriptor types
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // TODO: images/textures

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_deviceContext.getDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }
    }

    void Renderer::createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(m_deviceContext.getDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }

    void Renderer::createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(layouts.size());
        if (vkAllocateDescriptorSets(m_deviceContext.getDevice(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }
    }

    // void Renderer::updateDescriptorSets(VkImageView imageView) {
    //     for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    //         VkDescriptorBufferInfo bufferInfo{};
    //         bufferInfo.buffer = m_uniformBuffers[i];
    //         bufferInfo.offset = 0;
    //         bufferInfo.range = sizeof(UniformBufferObject);

    //         VkDescriptorImageInfo imageInfo{};
    //         imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //         imageInfo.imageView = imageView;
    //         imageInfo.sampler = m_textureSampler;

    //         std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    //         descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //         descriptorWrites[0].dstSet = m_descriptorSets[i];
    //         descriptorWrites[0].dstBinding = 0;
    //         descriptorWrites[0].dstArrayElement = 0;
    //         descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //         descriptorWrites[0].descriptorCount = 1;
    //         descriptorWrites[0].pBufferInfo = &bufferInfo;
    //         descriptorWrites[0].pImageInfo = nullptr; // TODO
    //         descriptorWrites[0].pTexelBufferView = nullptr; // TODO

    //         descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //         descriptorWrites[1].dstSet = m_descriptorSets[i];
    //         descriptorWrites[1].dstBinding = 1;
    //         descriptorWrites[1].dstArrayElement = 0;
    //         descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //         descriptorWrites[1].descriptorCount = 1;
    //         descriptorWrites[1].pImageInfo = &imageInfo;
    //         descriptorWrites[1].pBufferInfo = nullptr; // TODO
    //         descriptorWrites[1].pTexelBufferView = nullptr; // TODO

    //         vkUpdateDescriptorSets(m_deviceContext.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    //     }
    // }   
 
    // TODO: move to command manager
    void Renderer::copyAllocation(Allocation& alloc) {
        VkCommandBuffer cmdBuffer;
        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_commandManager_ptr->getTransferCommandPool(),
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        vkAllocateCommandBuffers(m_deviceContext.getDevice(), &allocInfo, &cmdBuffer);

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        vkBeginCommandBuffer(cmdBuffer, &beginInfo);

        m_bufferManager_ptr->copyBuffer(&alloc.indexStaging, m_bufferManager_ptr->getIndexBufferPtr().get(), VkBufferCopy{.srcOffset = 0, .dstOffset = alloc.indexOffset, .size = alloc.indexSize});
        m_bufferManager_ptr->copyBuffer(&alloc.vertexStaging, m_bufferManager_ptr->getIndexBufferPtr().get(), VkBufferCopy{.srcOffset = 0, .dstOffset = alloc.vertexOffset, .size = alloc.vertexSize});

        vkEndCommandBuffer(cmdBuffer);
        VkFence fence;
        VkFenceCreateInfo fenceInfo = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        vkCreateFence(m_deviceContext.getDevice(), &fenceInfo, nullptr, &fence);

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdBuffer
        };
        vkQueueSubmit(m_deviceContext.getTransferQueue(), 1, &submitInfo, fence);
        vkWaitForFences(m_deviceContext.getDevice(), 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(m_deviceContext.getDevice(), fence, nullptr);

        vkFreeCommandBuffers(m_deviceContext.getDevice(), m_commandManager_ptr->getTransferCommandPool(), 1, &cmdBuffer);
    }

    void Renderer::recordCommandBuffer(std::vector<MeshComponent> meshes, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_frameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchainContext.getExtent();

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapchainContext.getExtent().width);
        viewport.height = static_cast<float>(m_swapchainContext.getExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swapchainContext.getExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Bind Descriptors
        // TODO: implement better way than `imageIndex % MAX_FRAMES_IN_FLIGHT`
        // std::cout << "Binding descriptor set: " << m_descriptorSets[imageIndex % MAX_FRAMES_IN_FLIGHT] << " for image " << imageIndex % MAX_FRAMES_IN_FLIGHT << std::endl;
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipelineLayout, 0, 1, &m_descriptorSets[imageIndex % MAX_FRAMES_IN_FLIGHT], 0, nullptr);

        Material* currentMaterial = nullptr;
        for (AetherEngine::Rendering::MeshComponent mesh : meshes) {
            if (mesh.getMaterialPtr().get() != currentMaterial) {
                currentMaterial = mesh.getMaterialPtr().get();
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
                auto descriptorSet = currentMaterial->getDescriptorSet();
                vkCmdBindDescriptorSets(
                    commandBuffer, 
                    VK_PIPELINE_BIND_POINT_GRAPHICS, 
                    m_graphicsPipelineLayout,
                    1, 
                    1, 
                    &descriptorSet, 
                    0,
                    // &mesh.dynamicUBOIndex * bufferManager.getDynamicUBOAlignment());
                    nullptr
                );
            }

            // Bind buffers
            VkDeviceSize vertexOffset[] = { mesh.getVertexOffset() };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_bufferManager_ptr->getVertexBufferPtr()->buffer, vertexOffset);
            vkCmdBindIndexBuffer(commandBuffer, m_bufferManager_ptr->getIndexBufferPtr()->buffer, mesh.getIndexOffset(), VK_INDEX_TYPE_UINT32);

            // vkCmdDraw(commandBuffer,  static_cast<uint32_t>(m_vertices.size()), 1, 0, 0);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.getVertices().size()), 1, 0, 0, 0);
        }


        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    
    
    void Renderer::drawFrame(std::vector<MeshComponent> meshes) {
        static size_t currentFrame = 0;

        vkWaitForFences(m_deviceContext.getDevice(), 1, &m_commandManager_ptr->getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(m_deviceContext.getDevice(), 1, &m_commandManager_ptr->getInFlightFences()[currentFrame]);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_deviceContext.getDevice(), m_swapchainContext.getSwapchain(), UINT64_MAX, m_commandManager_ptr->getImageAvailableSemaphores()[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // m_swapchainContext.recreateSwapchain();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        vkResetCommandBuffer(m_commandManager_ptr->getCommandBuffers()[imageIndex], 0);
        recordCommandBuffer(meshes, m_commandManager_ptr->getCommandBuffers()[imageIndex], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_commandManager_ptr->getImageAvailableSemaphores()[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandManager_ptr->getCommandBuffers()[imageIndex];

        VkSemaphore signalSemaphores[] = {m_commandManager_ptr->getRenderFinishedSemaphores()[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_deviceContext.getGraphicsQueue(), 1, &submitInfo, m_commandManager_ptr->getInFlightFences()[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {m_swapchainContext.getSwapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_deviceContext.getPresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // Handle swapchain recreation
            m_swapchainContext.recreateSwapchain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
    
    
}