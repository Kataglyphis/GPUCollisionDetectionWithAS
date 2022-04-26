#include "ParticleRenderStage.h"
#include "Vertex.h"
#include "PipelineToolKit.h"
#include "Utilities.h"
#include "DeferredParticleSubpass.h"



ParticleRenderStage::ParticleRenderStage(Context* context, DescriptorSet* descriptorSet, RenderAttachmentInfo attachementInfo, uint32_t subpassIndex)
{
    this->context = context;
    this->descriptorSet = descriptorSet;
    this->subpass = new DeferredParticleSubpass(attachementInfo);
    this->subpassIndex = subpassIndex;
}

void ParticleRenderStage::destroyPipeline()
{
    vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
    vkDestroyPipeline(context->device, pipeline, nullptr);
}

ParticleRenderStage::~ParticleRenderStage()
{
}

void ParticleRenderStage::destroyShaders()
{
    vkDestroyShaderModule(context->device, shaderModuleVert, nullptr);
    vkDestroyShaderModule(context->device, shaderModuleGeom, nullptr);
    vkDestroyShaderModule(context->device, shaderModuleFrag, nullptr);
}

void ParticleRenderStage::compileShaders()
{
    compileShader(context->device, shaders.vert, &shaderModuleVert);
    compileShader(context->device, shaders.geom, &shaderModuleGeom);
    compileShader(context->device, shaders.frag, &shaderModuleFrag);
}

void ParticleRenderStage::createPipeline(VkRenderPass renderPass)
{
    compileShaders();
    vk_init::PipelineConfiguration conf = {};
    conf.shaderModules = { shaderModuleVert, shaderModuleGeom ,shaderModuleFrag };
    conf.shaderStages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
    conf.vertexBindingDescription = { Particle::getBindingDescription() };
    conf.vertexAttributeDescriptions = { Particle::getAttributeDescriptions() };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    conf.blendAttachments.push_back(colorBlendAttachment);
    conf.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    conf.width = context->width;
    conf.height = context->height;
    conf.renderPass = renderPass;
    conf.pDescriptorSetLayout = &descriptorSet->layout;
    conf.subpassIndex = subpassIndex;
    conf.enableDepthTest = VK_TRUE;
    conf.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    vk_init::createPipeline(context->device, conf, &pipelineLayout, &pipeline);
    destroyShaders();
}

void ParticleRenderStage::recordRenderCommands(VkCommandBuffer& cmdBuf, VkBuffer* pParticleBuffer, uint32_t particleCount)
{

    


    VkDeviceSize offsets[] = { 0 };
    vkCmdNextSubpass(cmdBuf, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, pParticleBuffer, offsets);
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);
    vkCmdDraw(cmdBuf, particleCount, 1, 0, 0);
}

void ParticleRenderStage::recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params)
{
    /*VkDeviceSize offsets[] = { 0 };
    vkCmdNextSubpass(cmdBuf, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, params.pVertexBufferParticles, offsets);
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);
    vkCmdDraw(cmdBuf, params.vertexCountParticles, 1, 0, 0);*/

    VkDeviceSize offsets[] = { 0 };
    vkCmdNextSubpass(cmdBuf, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, params.pVertexBufferParticles, offsets);
    //vkCmdBindVertexBuffers(cmdBuf, 0, 1, params.pVertexBufferSceneParticles, offsets);
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);
    vkCmdDraw(cmdBuf, params.vertexCountParticles, 1, 0, 0);
    //vkCmdDraw(cmdBuf, params.vertexCountSceneParticles, 1, 0, 0);
}

Subpass* ParticleRenderStage::getSubpass()
{
    return subpass;
}

