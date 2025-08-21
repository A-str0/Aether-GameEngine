#define GLM_FORCE_RADIANS

#include "renderer.h"

#include <stdexcept>
#include <array>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include <chrono>

#include <glm/gtc/matrix_transform.hpp>

#include "../objects/vertex.h"

// TODO: REFACTOR!!!
#include <stb/stb_image.h>

#define MAX_FRAMES_IN_FLIGHT 2

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
        VkSurfaceKHR surface
    ) : m_deviceContext(deviceContext), m_swapchainContext(swapchainContext) {

        createRenderPass();
        createShaderModules();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createFramebuffers();
        createTransferCommandPool();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();

        loadImage();
        createTextureImageView();
        createTextureSampler();

        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        
    }

    Renderer::~Renderer() {
        VkDevice device = m_deviceContext.getDevice();
        for (auto semaphore : m_imageAvailableSemaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
        for (auto semaphore : m_renderFinishedSemaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
        for (auto fence : m_inFlightFences) {
            vkDestroyFence(device, fence, nullptr);
        }
        for (auto framebuffer : m_frameBuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, m_uniformBuffers[i], nullptr);
            vkFreeMemory(device, m_uniformBuffersMemory[i], nullptr);
        }

        vkDestroySampler(device, m_textureSampler, nullptr);
        vkDestroyImageView(device, m_textureImageView, nullptr);
        vkDestroyImage(device, m_textureImage, nullptr);
        vkFreeMemory(device, m_textureImageMemory, nullptr);

        vkDestroyBuffer(device, m_vertexBuffer, nullptr);
        vkFreeMemory(device, m_vertexBufferMemory, nullptr);
        vkDestroyBuffer(device, m_indexBuffer, nullptr);
        vkFreeMemory(device, m_indexBufferMemory, nullptr);

        vkDestroyCommandPool(device, m_commandPool, nullptr);
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
        auto bindingDescription = Objects::Vertex::getBindingDescription();
        auto attributeDescriptions = Objects::Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.pNext = nullptr;
        vertexInput.vertexBindingDescriptionCount = 1;
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

    void Renderer::createVertexBuffer() {
        VkDeviceSize vertexBufferSize = sizeof(m_vertices[0]) * m_vertices.size();
        
        // Craete Staging Buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(
            vertexBufferSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer, 
            stagingBufferMemory
        );

        // TODO: refactor
        void* data; // TODO
        vkMapMemory(m_deviceContext.getDevice(), stagingBufferMemory, 0, vertexBufferSize, 0, &data);
            memcpy(data, m_vertices.data(), (size_t) vertexBufferSize);
        vkUnmapMemory(m_deviceContext.getDevice(), stagingBufferMemory);

        // Create VertexBuffer
        createBuffer(
            vertexBufferSize, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_vertexBuffer,
            m_vertexBufferMemory
        );

        // Copy Vertices
        copyBuffer(stagingBuffer, m_vertexBuffer, vertexBufferSize);

        // Cleanup
        vkDestroyBuffer(m_deviceContext.getDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_deviceContext.getDevice(), stagingBufferMemory, nullptr);
    }

    void Renderer::createIndexBuffer() {
        VkDeviceSize indexBufferSize = sizeof(m_indices[0]) * m_indices.size();
        
        // Craete Staging Buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(
            indexBufferSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer, 
            stagingBufferMemory
        );

        // TODO: refactor
        void* data; // TODO
        vkMapMemory(m_deviceContext.getDevice(), stagingBufferMemory, 0, indexBufferSize, 0, &data);
            memcpy(data, m_indices.data(), (size_t) indexBufferSize);
        vkUnmapMemory(m_deviceContext.getDevice(), stagingBufferMemory);

        // Create IndexBuffer
        createBuffer(
            indexBufferSize, 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_indexBuffer,
            m_indexBufferMemory
        );

        // Copy Indices
        copyBuffer(stagingBuffer, m_indexBuffer, indexBufferSize);

        // Cleanup
        vkDestroyBuffer(m_deviceContext.getDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_deviceContext.getDevice(), stagingBufferMemory, nullptr);
    }

    void Renderer::createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(
                bufferSize, 
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                m_uniformBuffers[i], 
                m_uniformBuffersMemory[i]
            );

            vkMapMemory(m_deviceContext.getDevice(), m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_uniformBuffersMapped[i]);
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

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_textureImageView;
            imageInfo.sampler = m_textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[0].pImageInfo = nullptr; // TODO
            descriptorWrites[0].pTexelBufferView = nullptr; // TODO

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
            descriptorWrites[1].pBufferInfo = nullptr; // TODO
            descriptorWrites[1].pTexelBufferView = nullptr; // TODO


            vkUpdateDescriptorSets(m_deviceContext.getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.flags = 0; // TODO
        bufferInfo.pNext = nullptr; // TODO
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT; // is it??

        if (vkCreateBuffer(m_deviceContext.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer!");
        }

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(m_deviceContext.getDevice(), buffer, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = m_deviceContext.getMemoryType(memoryRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_deviceContext.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_deviceContext.getDevice(), buffer, bufferMemory, 0);
    }

    void Renderer::createTransferCommandPool() {
        // Create Transfer Command Pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_deviceContext.getTransferFamily();

        if (vkCreateCommandPool(m_deviceContext.getDevice(), &poolInfo, nullptr, &m_transferCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create transfer command pool!");
        }
    }

    void Renderer::createCommandPool() {
        // Create Command Pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_deviceContext.getGraphicsFamily();

        if (vkCreateCommandPool(m_deviceContext.getDevice(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void Renderer::createCommandBuffers() {
        // Allocate Command Buffer
        auto imageCount = m_swapchainContext.getImageViews().size();
        m_commandBuffers.resize(imageCount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_deviceContext.getDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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

        // Bind VertexBuffer
        VkBuffer vertexBuffers[] = {m_vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        // Bind IndexBuffer
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        // Bind Descriptors
        // TODO: implement better way than `imageIndex % MAX_FRAMES_IN_FLIGHT`
        // std::cout << "Binding descriptor set: " << m_descriptorSets[imageIndex % MAX_FRAMES_IN_FLIGHT] << " for image " << imageIndex % MAX_FRAMES_IN_FLIGHT << std::endl;
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipelineLayout, 0, 1, &m_descriptorSets[imageIndex % MAX_FRAMES_IN_FLIGHT], 0, nullptr);

        // vkCmdDraw(commandBuffer,  static_cast<uint32_t>(m_vertices.size()), 1, 0, 0);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    VkCommandBuffer Renderer::beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        // TODO: make cooler memory allocation (don't call vkAllocateCommandBuffers for every buffer)
        vkAllocateCommandBuffers(m_deviceContext.getDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // TODO: change Queue to submit
        vkQueueSubmit(m_deviceContext.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        // TODO: fence usage
        vkQueueWaitIdle(m_deviceContext.getGraphicsQueue());

        vkFreeCommandBuffers(m_deviceContext.getDevice(), m_commandPool, 1, &commandBuffer);
    }

    void Renderer::updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_swapchainContext.getExtent().width / (float) m_swapchainContext.getExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        // Using a UBO this way is not the most efficient way to pass frequently changing values to the shader. 
        // A more efficient way to pass a small buffer of data to shaders are push constants
        memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void Renderer::createSyncObjects() {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(m_deviceContext.getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_deviceContext.getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_deviceContext.getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create synchronization objects!");
            }
        }
    }
    
    void Renderer::drawFrame() {
        static size_t currentFrame = 0;

        vkWaitForFences(m_deviceContext.getDevice(), 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(m_deviceContext.getDevice(), 1, &m_inFlightFences[currentFrame]);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_deviceContext.getDevice(), m_swapchainContext.getSwapchain(), UINT64_MAX, m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // m_swapchainContext.recreateSwapchain();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);
        recordCommandBuffer(m_commandBuffers[imageIndex], imageIndex);

        updateUniformBuffer(currentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_deviceContext.getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[currentFrame]) != VK_SUCCESS) {
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
    
    // void Renderer::loadImage(int &width, int &height, VkDeviceSize imageSize, u_char* imageData, void(*func_ptr)(u_char*)) {
    void Renderer::loadImage() {
        // TODO: REFACTOR!!! 
        int width, height, channels;
        stbi_uc* pixels = stbi_load("../../../src/core/rendering/textures/tex.jpg", &width, &height, &channels, STBI_rgb_alpha);
        VkDeviceSize imageSize = width * height * 4; // is it??

        if (!pixels) {
            throw std::runtime_error("Failed to load texture image!");
        }
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        createBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer, 
            stagingBufferMemory
        );

        void* data;
        vkMapMemory(m_deviceContext.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_deviceContext.getDevice(), stagingBufferMemory);

        // func_ptr(imageData);
        stbi_image_free(pixels);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(width);
        imageInfo.extent.height = static_cast<uint32_t>(height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB; // TODO: custom selection
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // TODO

        if (vkCreateImage(m_deviceContext.getDevice(), &imageInfo, nullptr, &m_textureImage) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_deviceContext.getDevice(), m_textureImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_deviceContext.getMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(m_deviceContext.getDevice(), &allocInfo, nullptr, &m_textureImageMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate image memory!");
        }

        vkBindImageMemory(m_deviceContext.getDevice(), m_textureImage, m_textureImageMemory, 0);

        transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(m_deviceContext.getDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_deviceContext.getDevice(), stagingBufferMemory, nullptr);
    }

    void Renderer::createTextureImageView() {
        m_textureImageView = m_swapchainContext.createImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    }

    void Renderer::createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_deviceContext.getPhysicalDevice(), &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(m_deviceContext.getDevice(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0; // TODO
        barrier.dstAccessMask = 0; // TODO

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(commandBuffer);
    }

    void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endSingleTimeCommands(commandBuffer);
    }
}