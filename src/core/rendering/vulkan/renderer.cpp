#include "renderer.h"
#include <stdexcept>
#include <array>
#include <fstream>
#include <vector>

namespace AetherEngine::Rendering {
    std::vector<char> readShaderFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + filename);
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    Renderer::Renderer(
        VulkanDeviceContext& deviceContext, 
        VulkanSwapchainContext& swapchainContext, 
        VkSurfaceKHR surface
    ) : m_deviceContext(m_deviceContext), m_swapchainContext(swapchainContext) {
        
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
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.pNext = nullptr;
        renderPassCreateInfo.flags = 0x0;
        renderPassCreateInfo.attachmentCount = 1; // TODO: change to auto
        renderPassCreateInfo.pAttachments = &colorAttachment; // TODO: change
        renderPassCreateInfo.subpassCount = 1; // TODO: change to auto
        renderPassCreateInfo.pSubpasses = &graphicsSubpass; // TODO: change
        renderPassCreateInfo.dependencyCount = 0; // TODO
        renderPassCreateInfo.pDependencies = nullptr; // TODO

        VkRenderPass m_renderPass{}; // TODO: move to fileds
        vkCreateRenderPass2(m_deviceContext.getDevice(), &renderPassCreateInfo, nullptr, &m_renderPass);

        // Create ShaderModules
        // Create Vertex ShaderModule
        const std::vector<char> vertexCode = readShaderFile("../compiled_shaders/triangle_vert.spv"); // TODO: normal directory finding

        VkShaderModuleCreateInfo vertexShaderModuleCreateInfo{};
        vertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertexShaderModuleCreateInfo.pNext = nullptr; // TODO
        // vertexShaderModuleCreateInfo.flags = // TODO???
        vertexShaderModuleCreateInfo.codeSize = vertexCode.size();
        vertexShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexCode.data()); // ???

        vkCreateShaderModule(deviceContext.getDevice(), &vertexShaderModuleCreateInfo, nullptr, &m_vertexShaderModule);

        // Create Fragment ShaderModule
        const std::vector<char> fragmentCode = readShaderFile("../compiled_shaders/triangle_frag.spv"); // TODO: normal directory finding

        VkShaderModuleCreateInfo fragmentShaderModuleCreateInfo{};
        fragmentShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragmentShaderModuleCreateInfo.pNext = nullptr; // TODO
        // fragmentShaderModuleCreateInfo.flags = // TODO???
        fragmentShaderModuleCreateInfo.codeSize = fragmentCode.size();
        fragmentShaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentCode.data()); // ???

        vkCreateShaderModule(deviceContext.getDevice(), &fragmentShaderModuleCreateInfo, nullptr, &m_fragmentShaderModule);

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
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.pNext = nullptr;
        vertexInputInfo.vertexBindingDescriptionCount = 0; // TODO
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // TODO
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        // Create InputAssembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Create Viewport
        // VkViewport viewport{};
        // viewport.x = 0.0f;
        // viewport.y = 0.0f;
        // viewport.width = (float)m_swapchainContext.getExtent().width; // TODO: normal cast
        // viewport.height = (float)m_swapchainContext.getExtent().height; // TODO: normal cast
        // viewport.minDepth = 0.0f;
        // viewport.maxDepth = 1.0f;

        // // Create Scissors
        // VkRect2D scissor{};
        // scissor.offset = {0, 0};
        // scissor.extent = m_swapchainContext.getExtent();

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
    }

    Renderer::~Renderer() {
        if (m_fragmentShaderModule) {
            vkDestroyShaderModule(m_deviceContext.getDevice(), m_fragmentShaderModule, nullptr);
        }
        if (m_vertexShaderModule) {
            vkDestroyShaderModule(m_deviceContext.getDevice(), m_vertexShaderModule, nullptr);
        }
    }
}