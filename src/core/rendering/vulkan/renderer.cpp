#define GLM_FORCE_RADIANS

#include "renderer.hpp"

#include <stdexcept>
#include <array>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include <chrono>

#include <glm/gtc/matrix_transform.hpp>

#include <rendering/objects/vertex.hpp>
#include <resource_managment/objects/texture_resource.hpp>

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
            VulkanDeviceContext* deviceContext_ptr, 
            VulkanSwapchainContext* swapchainContext_ptr, 
            MemoryManager* memoryManager_ptr,
            VkSurfaceKHR surface
        ) : 
            m_deviceContext_ptr(deviceContext_ptr), 
            m_swapchainContext_ptr(swapchainContext_ptr),
            m_memoryManager_ptr(memoryManager_ptr)
        {
            createRenderPass();
            // TODO: createRenderPass2();
            createShaderModules();
            createGraphicsPipeline();
            createFramebuffers();
            createTextureSampler();
            createSyncObjects();
            // createDescriptorPool();

            // createTransferCommandPool();
            // createCommandPool();
            // createCommandBuffers();
            // createSyncObjects();

            // createDescriptorSetLayout();
            // createDescriptorSets();

            // createUniformBuffers();
        }

    Renderer::~Renderer() {
        VkDevice device = m_deviceContext_ptr->getDevice();
        // for (auto semaphore : m_imageAvailableSemaphores) {
        //     vkDestroySemaphore(device, semaphore, nullptr);
        // }
        // for (auto semaphore : m_renderFinishedSemaphores) {
        //     vkDestroySemaphore(device, semaphore, nullptr);
        // }
        // for (auto fence : m_inFlightFences) {
        //     vkDestroyFence(device, fence, nullptr);
        // }
        // for (auto framebuffer : m_frameBuffers) {
        //     vkDestroyFramebuffer(device, framebuffer, nullptr);
        // }
        // for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        //     vkDestroyBuffer(device, m_uniformBuffers[i], nullptr);
        //     vkFreeMemory(device, m_uniformBuffersMemory[i], nullptr);
        // }

        vkDestroySampler(device, m_textureSampler, nullptr);

        // Note: Vertex and index buffers are now managed by individual meshes
        // vkDestroyBuffer(device, m_vertexBuffer, nullptr);
        // vkFreeMemory(device, m_vertexBufferMemory, nullptr);
        // vkDestroyBuffer(device, m_indexBuffer, nullptr);
        // vkFreeMemory(device, m_indexBufferMemory, nullptr);

        // vkDestroyCommandPool(device, m_commandPool, nullptr);

        // vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
        // vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);

        vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, m_graphicsPipelineLayout, nullptr);

        vkDestroyShaderModule(device, m_vertexShaderModule, nullptr);
        vkDestroyShaderModule(device, m_fragmentShaderModule, nullptr);

        vkDestroyRenderPass(device, m_renderPass, nullptr);
    }

    void Renderer::drawFrame(const std::vector<Objects::Mesh*> meshes) {
        static size_t currentFrame = 0;

        vkWaitForFences(m_deviceContext_ptr->getDevice(), 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(m_deviceContext_ptr->getDevice(), 1, &m_inFlightFences[currentFrame]);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_deviceContext_ptr->getDevice(), m_swapchainContext_ptr->getSwapchain(), UINT64_MAX, m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // m_swapchainContext_ptr->recreateSwapchain();
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        const auto commandBuffers = m_memoryManager_ptr->getCommandBuffers();
        vkResetCommandBuffer(commandBuffers[imageIndex], 0);
        recordCommandBuffer(commandBuffers[imageIndex], imageIndex, meshes);

        m_memoryManager_ptr->updateUniformBuffer(currentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_deviceContext_ptr->getGraphicsQueue(), 1, &submitInfo, m_inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {m_swapchainContext_ptr->getSwapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_deviceContext_ptr->getPresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            // m_swapchainContext_ptr->recreateSwapchain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapchainContext_ptr->getFormat();
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

        if (vkCreateRenderPass(m_deviceContext_ptr->getDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
        
    }

    // TODO: use this
    void Renderer::createRenderPass2() {
        // Create attachments
        // Create Color Attachment
        VkAttachmentDescription2 colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
        colorAttachment.pNext = nullptr;
        // colorAttachment.flags = // TODO
        colorAttachment.format = m_swapchainContext_ptr->getFormat();
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
        vkCreateRenderPass2(m_deviceContext_ptr->getDevice(), &renderPassCreateInfo, nullptr, &m_renderPass);
    }

    void Renderer::createShaderModules() {
        // Create Vertex ShaderModule
        try {
            const std::vector<char> vertexCode = readShaderFile("../../../src/core/rendering/compiled_shaders/standard_vertex_shader.spv");
            // const std::vector<char> vertexCode = readShaderFile("src/core/rendering/compiled_shaders/standard_vertex_shader.spv");
            
            VkShaderModuleCreateInfo vertexShaderModuleCreateInfo{};
            vertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            vertexShaderModuleCreateInfo.codeSize = vertexCode.size();
            vertexShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexCode.data());

            if (vkCreateShaderModule(m_deviceContext_ptr->getDevice(), &vertexShaderModuleCreateInfo, nullptr, &m_vertexShaderModule) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create vertex shader module!");
            }
            std::cout << "Vertex shader module created successfully\n";
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to create vertex shader: " + std::string(e.what()));
        }

        // Create Fragment ShaderModule 
        try {
            const std::vector<char> fragmentCode = readShaderFile("../../../src/core/rendering/compiled_shaders/standard_fragment_shader.spv");
            // const std::vector<char> fragmentCode = readShaderFile("src/core/rendering/compiled_shaders/standard_fragment_shader.spv");
            
            VkShaderModuleCreateInfo fragmentShaderModuleCreateInfo{};
            fragmentShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            fragmentShaderModuleCreateInfo.codeSize = fragmentCode.size();
            fragmentShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentCode.data());

            if (vkCreateShaderModule(m_deviceContext_ptr->getDevice(), &fragmentShaderModuleCreateInfo, nullptr, &m_fragmentShaderModule) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create fragment shader module!");
            }
            std::cout << "Fragment shader module created successfully\n";
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to create fragment shader: " + std::string(e.what()));
        }
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

        auto bindingDescription = Objects::Vertex::getBindingDescription();
        auto attributeDescriptions = Objects::Vertex::getAttributeDescriptions();
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDescription;
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
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = m_memoryManager_ptr->getDescriptorSetLayout();
        pipelineLayoutInfo.pushConstantRangeCount = 0; // TODO
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // TODO

        if (vkCreatePipelineLayout(m_deviceContext_ptr->getDevice(), &pipelineLayoutInfo, nullptr, &m_graphicsPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        // Validate shader modules and pipeline layout with detailed debug
        std::cout << "Validating pipeline components...\n";
        std::cout << "Vertex shader module: " << m_vertexShaderModule << "\n";
        std::cout << "Fragment shader module: " << m_fragmentShaderModule << "\n";
        std::cout << "Pipeline layout: " << m_graphicsPipelineLayout << "\n";
        std::cout << "Render pass: " << m_renderPass << "\n";
        
        if (m_vertexShaderModule == VK_NULL_HANDLE) {
            throw std::runtime_error("Vertex shader module is not valid!");
        }
        if (m_fragmentShaderModule == VK_NULL_HANDLE) {
            throw std::runtime_error("Fragment shader module is not valid!");
        }
        if (m_graphicsPipelineLayout == VK_NULL_HANDLE) {
            throw std::runtime_error("Pipeline layout is not valid!");
        }
        if (m_renderPass == VK_NULL_HANDLE) {
            throw std::runtime_error("Render pass is not valid!");
        }
        std::cout << "All pipeline components validated successfully\n";

        // Create Graphics Pipeline
        VkGraphicsPipelineCreateInfo graphicsPiplineInfo{};
        graphicsPiplineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPiplineInfo.stageCount = 2; // TODO: dynamic
        graphicsPiplineInfo.pStages = shaderStages;

        graphicsPiplineInfo.pVertexInputState = &vertexInput;
        graphicsPiplineInfo.pInputAssemblyState = &inputAssembly;
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
        graphicsPiplineInfo.basePipelineIndex = -1; // TODO

        std::cout << "About to create graphics pipeline...\n";
        if (vkCreateGraphicsPipelines(m_deviceContext_ptr->getDevice(), VK_NULL_HANDLE, 1, &graphicsPiplineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }
        std::cout << "Graphics pipeline created successfully!\n";
    }

    void Renderer::createFramebuffers() {
        std::vector<VkImageView> imageViews = m_swapchainContext_ptr->getImageViews();
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
            framebufferInfo.height = m_swapchainContext_ptr->getExtent().height;
            framebufferInfo.width = m_swapchainContext_ptr->getExtent().width;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_deviceContext_ptr->getDevice(), &framebufferInfo, nullptr, &m_frameBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
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
        vkGetPhysicalDeviceProperties(m_deviceContext_ptr->getPhysicalDevice(), &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(m_deviceContext_ptr->getDevice(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
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
            if (vkCreateSemaphore(m_deviceContext_ptr->getDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_deviceContext_ptr->getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_deviceContext_ptr->getDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create synchronization objects!");
            }
        }
    }

    void Renderer::updateDescriptorSets(VkImageView imageView) {
        auto uniformBuffers = m_memoryManager_ptr->getUniformBuffers();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = imageView;
            imageInfo.sampler = m_textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            const std::vector<VkDescriptorSet> descriptorSets = m_memoryManager_ptr->getDescriptorSets();
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[0].pImageInfo = nullptr; // TODO
            descriptorWrites[0].pTexelBufferView = nullptr; // TODO

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
            descriptorWrites[1].pBufferInfo = nullptr; // TODO
            descriptorWrites[1].pTexelBufferView = nullptr; // TODO

            vkUpdateDescriptorSets(m_deviceContext_ptr->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }   

    void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const std::vector<Objects::Mesh*> meshes) {
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
        renderPassInfo.renderArea.extent = m_swapchainContext_ptr->getExtent();

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapchainContext_ptr->getExtent().width);
        viewport.height = static_cast<float>(m_swapchainContext_ptr->getExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swapchainContext_ptr->getExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Render all meshes using unified buffers and offsets
        VkBuffer vertexBuffer = m_memoryManager_ptr->getVertexBuffer();
        VkBuffer indexBuffer = m_memoryManager_ptr->getIndexBuffer();
        
        for (const auto& mesh : meshes) {
            // Bind vertex buffer with offset
            VkDeviceSize vertexOffset = mesh->getVertexOffset();
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &vertexOffset);
            
            // Bind index buffer with offset
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, mesh->getIndexOffset(), VK_INDEX_TYPE_UINT16);
            
            // Bind descriptor sets
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipelineLayout, 0, 1, &m_memoryManager_ptr->getDescriptorSets()[imageIndex % MAX_FRAMES_IN_FLIGHT], 0, nullptr);

            // Draw indexed
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }
}