#include "DeferredShadingRenderStage.h"
#include "Vertex.h"
#include "PipelineToolKit.h"
#include "Utilities.h"
#include "DeferredShadingSubpass.h"



DeferredShadingRenderStage::DeferredShadingRenderStage(Context* context, DescriptorSet* descriptorSet, RenderAttachmentInfo attachementInfo, uint32_t subpassIndex)
{
    this->context = context;
    this->descriptorSet = descriptorSet;
    this->subpass = new DeferredShadingSubpass(attachementInfo);
    this->subpassIndex = subpassIndex;
}

void DeferredShadingRenderStage::destroyPipeline()
{
    vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
    vkDestroyPipeline(context->device, pipeline, nullptr);
}

DeferredShadingRenderStage::~DeferredShadingRenderStage()
{
}

void DeferredShadingRenderStage::destroyShaders()
{
    vkDestroyShaderModule(context->device, shaderModuleVert, nullptr);
    vkDestroyShaderModule(context->device, shaderModuleFrag, nullptr);
}

void DeferredShadingRenderStage::compileShaders()
{
    compileShader(context->device, shaders.vert, &shaderModuleVert);
    compileShader(context->device, shaders.frag, &shaderModuleFrag);
}

void DeferredShadingRenderStage::createPipeline(VkRenderPass renderPass)
{
    compileShaders();
    vk_init::PipelineConfiguration conf = {};
    conf.shaderModules = { shaderModuleVert, shaderModuleFrag };
    conf.shaderStages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
    conf.vertexBindingDescription = {};
    conf.vertexAttributeDescriptions = {};

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    conf.blendAttachments.push_back(colorBlendAttachment);
    conf.pushConstantRanges.push_back({VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants)});

    conf.width = context->width;
    conf.height = context->height;
    conf.renderPass = renderPass;
    conf.pDescriptorSetLayout = &descriptorSet->layout;
    conf.subpassIndex = subpassIndex;

    vk_init::createPipeline(context->device, conf, &pipelineLayout, &pipeline);
    destroyShaders();
}

void DeferredShadingRenderStage::recordRenderCommands(VkCommandBuffer& cmdBuf)
{
    vkCmdNextSubpass(cmdBuf, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);
    vkCmdDraw(cmdBuf, 3, 1, 0, 0); // Positions and UVs generated in vertex shader. (see also https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/)
}

void DeferredShadingRenderStage::recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params)
{
    vkCmdNextSubpass(cmdBuf, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);
    pc.enablePathTracing = g_enable_path_tracing.val.bool32();
    vkCmdPushConstants(cmdBuf, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DeferredShadingRenderStage::PushConstants), &pc.enablePathTracing);
    vkCmdDraw(cmdBuf, 3, 1, 0, 0); // Positions and UVs generated in vertex shader. (see also https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/)
}

Subpass* DeferredShadingRenderStage::getSubpass()
{
    return subpass;
}
