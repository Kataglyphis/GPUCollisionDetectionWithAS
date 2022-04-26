#pragma once
#include <vulkan/vulkan.h>
#include "Subpass.h"

// Vulkan Structs filled with default values
namespace vk_default
{
    static VkImageCreateInfo renderTargetImageCreateInfo()
    {
        VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.mipLevels = 1;
        return imageCreateInfo;
    }

    static VkImageViewCreateInfo renderTargetImageViewCreateInfo()
    {
        VkImageViewCreateInfo imageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        return imageViewCreateInfo;
    }
    static VkAttachmentDescription renderTargetAttchementDescription()
    {
        VkAttachmentDescription attachementDescription = {};
        attachementDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachementDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachementDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachementDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachementDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachementDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachementDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        return attachementDescription;
    }

    static VkImageCreateInfo textureImageCreateInfo()
    {
        VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        return imageCreateInfo;
    }

    // Will add funktionality to texture manager later
    static VkSamplerCreateInfo textureSamplerCreateInfo()
    {
        VkSamplerCreateInfo samplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.mipLodBias = 0.0;
        samplerCreateInfo.anisotropyEnable = VK_FALSE;
        samplerCreateInfo.maxAnisotropy = 1.0;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.minLod = 0.0;
        samplerCreateInfo.maxLod = 12.0; // CIV
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        return samplerCreateInfo;
    }



}

// Initialiser funtions
namespace vk_init
{
    static void createRenderPass(VkDevice device, std::vector<VkAttachmentDescription> attachments, std::vector<Subpass*> subpasses, VkRenderPass& renderpass)
    {
        std::vector<VkSubpassDescription> subpassDescriptions;
        std::vector<VkSubpassDependency> subpassDependencies;
        // Fill descriptions and dependencies
        for (uint32_t i = 0; i < subpasses.size(); i++)
        {
            subpassDescriptions.push_back(subpasses[i]->getSubpassDescription());
            subpassDependencies.push_back(subpasses[i]->getSubpassDependency(i));
        }
        subpassDependencies.push_back(Subpass::getFinalSubpassDependency());

        VkRenderPassCreateInfo renderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = subpassDescriptions.size();
        renderPassCreateInfo.pSubpasses = subpassDescriptions.data();
        renderPassCreateInfo.dependencyCount = subpassDependencies.size();
        renderPassCreateInfo.pDependencies = subpassDependencies.data();

        VkResult result = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderpass);
        ASSERT_VULKAN(result);
    }

    static void createSampler(VkDevice device, uint32_t width, uint32_t height, VkSampler* pSampler)
    {
        uint32_t mipLevels = std::floor(std::log2(std::max(width, height))) + 1;
        VkSamplerCreateInfo smpCreateInfo = vk_default::textureSamplerCreateInfo();
        smpCreateInfo.maxLod = mipLevels;
        VkResult result = vkCreateSampler(device, &smpCreateInfo, nullptr, pSampler);
        ASSERT_VULKAN(result);
    }


    struct PipelineConfiguration
    {
        // Need to be filled
        std::vector<VkShaderModule>                         shaderModules;
        std::vector<VkShaderStageFlagBits>                  shaderStages;
        std::vector<VkVertexInputBindingDescription>        vertexBindingDescription;
        std::vector<VkVertexInputAttributeDescription>      vertexAttributeDescriptions;
        std::vector<VkPipelineColorBlendAttachmentState>    blendAttachments;
        uint32_t                                            width;
        uint32_t                                            height;
        VkRenderPass                                        renderPass;
        VkDescriptorSetLayout*                              pDescriptorSetLayout;
        // Can be left empty
        VkPrimitiveTopology                                 primitiveTopology                   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        uint32_t                                            subpassIndex;
        VkCullModeFlags                                     cullMode;
        VkCompareOp                                         depthCompareOp;
        VkBool32                                            enableDepthClamp;
        VkBool32                                            enableDiscard;
        VkBool32                                            enableDepthTest;
        VkBool32                                            enableDepthWrite;
        VkBool32                                            enableStencilTest;
        std::vector <VkPushConstantRange>                   pushConstantRanges;

    };

    static void createPipeline(VkDevice device, PipelineConfiguration conf, VkPipelineLayout* pPipelineLayout, VkPipeline* pPipeline)
    {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        for (uint32_t i = 0; i < conf.shaderModules.size(); i++)
        {
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
            shaderStageCreateInfo.stage = conf.shaderStages[i];
            shaderStageCreateInfo.module = conf.shaderModules[i];
            shaderStageCreateInfo.pName = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        vertexInputCreateInfo.vertexBindingDescriptionCount = conf.vertexBindingDescription.size();
        vertexInputCreateInfo.pVertexBindingDescriptions = conf.vertexBindingDescription.data();
        vertexInputCreateInfo.vertexAttributeDescriptionCount = conf.vertexAttributeDescriptions.size();
        vertexInputCreateInfo.pVertexAttributeDescriptions = conf.vertexAttributeDescriptions.data();



        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        inputAssemblyCreateInfo.topology = conf.primitiveTopology;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;



        VkViewport viewport = {};
        viewport.width = conf.width;
        viewport.height = conf.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.extent = { conf.width, conf.height };

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = &viewport;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &scissor;



        VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        rasterizationCreateInfo.depthClampEnable = conf.enableDepthClamp;
        rasterizationCreateInfo.rasterizerDiscardEnable = conf.enableDiscard;
        rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationCreateInfo.cullMode = conf.cullMode;//VK_CULL_MODE_BACK_BIT;
        rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationCreateInfo.lineWidth = 1.0f;


        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleCreateInfo.minSampleShading = 1.0f;
        multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleCreateInfo.alphaToOneEnable = VK_FALSE;



        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencilStateCreateInfo.depthTestEnable = conf.enableDepthTest;
        depthStencilStateCreateInfo.depthWriteEnable = conf.enableDepthWrite;
        depthStencilStateCreateInfo.depthCompareOp = conf.depthCompareOp;
        depthStencilStateCreateInfo.stencilTestEnable = conf.enableStencilTest;
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.minDepthBounds = 0.0f;
        depthStencilStateCreateInfo.maxDepthBounds = 1.0f;



        VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        colorBlendCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
        colorBlendCreateInfo.attachmentCount = conf.blendAttachments.size();
        colorBlendCreateInfo.pAttachments = conf.blendAttachments.data();


        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dynamicStateCreateInfo.dynamicStateCount = 2;
        dynamicStateCreateInfo.pDynamicStates = dynamicStates;


        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = conf.pDescriptorSetLayout;
        pipelineLayoutCreateInfo.pPushConstantRanges = conf.pushConstantRanges.data();
        pipelineLayoutCreateInfo.pushConstantRangeCount = conf.pushConstantRanges.size();

        VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, pPipelineLayout);
        ASSERT_VULKAN(result);

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        pipelineCreateInfo.stageCount = shaderStages.size();
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineCreateInfo.pTessellationState = nullptr;
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
        pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        pipelineCreateInfo.layout = *pPipelineLayout;
        pipelineCreateInfo.renderPass = conf.renderPass;
        pipelineCreateInfo.subpass = conf.subpassIndex;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex = -1;

        result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, pPipeline);
        ASSERT_VULKAN(result);
    }

    struct ComputePipelineConfiguration
    {
        VkShaderModule          shader;
        uint32_t                workGroupSize;
        uint32_t                pushConstantSize;
        VkDescriptorSetLayout*  pDescriptorSetLayout;
    };

    static void createPipeline(VkDevice device, ComputePipelineConfiguration conf, VkPipelineLayout* pPipelineLayout, VkPipeline* pPipeline)
    {
        std::vector<VkSpecializationMapEntry> specEntries;
        VkSpecializationMapEntry specEntry;
        specEntry.constantID = 0;
        specEntry.offset = 0;
        specEntry.size = sizeof(uint32_t);
        specEntries.push_back(specEntry);

        std::vector<uint32_t> specValues = { conf.workGroupSize };

        VkSpecializationInfo specInfo;
        specInfo.mapEntryCount = specEntries.size();
        specInfo.pMapEntries = specEntries.data();
        specInfo.dataSize = specValues.size() * sizeof(uint32_t);
        specInfo.pData = specValues.data();

        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = conf.pushConstantSize;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = conf.pDescriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, pPipelineLayout);
        ASSERT_VULKAN(result);

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStageCreateInfo.module = conf.shader;
        shaderStageCreateInfo.pName = "main";
        shaderStageCreateInfo.pSpecializationInfo = &specInfo;

        VkComputePipelineCreateInfo pipelineCreateInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        pipelineCreateInfo.stage = shaderStageCreateInfo;
        pipelineCreateInfo.layout = *pPipelineLayout;
        result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, pPipeline);
        ASSERT_VULKAN(result);
    }
}

