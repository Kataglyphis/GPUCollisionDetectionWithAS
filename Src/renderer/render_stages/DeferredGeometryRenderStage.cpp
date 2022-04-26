#include "DeferredGeometryRenderStage.h"
#include "Vertex.h"
#include "PipelineToolKit.h"
#include "Utilities.h"
#include "DeferredGeometrySubpass.h"
#include "DrawGeom.h"



DeferredGeometryRenderStage::DeferredGeometryRenderStage(ResourceManager* rm, DescriptorSet* descriptorSet, RenderAttachmentInfo attachementInfo, uint32_t subpassIndex)
{
    this->rm = rm;
    this->context = rm->context;
    this->descriptorSet = descriptorSet;
    this->subpass = new DeferredGeometrySubpass(attachementInfo);
    this->subpassIndex = subpassIndex;
}

void DeferredGeometryRenderStage::destroyPipeline()
{
    vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
    vkDestroyPipeline(context->device, pipeline, nullptr);
}

DeferredGeometryRenderStage::~DeferredGeometryRenderStage()
{

}

void DeferredGeometryRenderStage::compileShaders()
{
    compileShader(context->device, shaders.vert, &shaderModuleVert);
    compileShader(context->device, shaders.frag, &shaderModuleFrag);
}

void DeferredGeometryRenderStage::destroyShaders()
{
    vkDestroyShaderModule(context->device, shaderModuleVert, nullptr);
    vkDestroyShaderModule(context->device, shaderModuleFrag, nullptr);
}

void DeferredGeometryRenderStage::createPipeline(VkRenderPass renderPass)
{
    compileShaders();
    vk_init::PipelineConfiguration conf = {};
    conf.shaderModules = { shaderModuleVert, shaderModuleFrag };
    conf.shaderStages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
    conf.vertexBindingDescription = { Vertex::getBindingDescription() };
    conf.vertexAttributeDescriptions = { Vertex::getAttributeDescriptions() };

    for (size_t i = 0; i < 4; i++)
    {
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        conf.blendAttachments.push_back(colorBlendAttachment);
    }
    conf.width = context->width;
    conf.height = context->height;
    conf.renderPass = renderPass;
    conf.pDescriptorSetLayout = &descriptorSet->layout;

    conf.subpassIndex = subpassIndex;
    //conf.cullMode = VK_CULL_MODE_BACK_BIT;
    conf.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    conf.enableDepthTest = VK_TRUE;
    conf.enableDepthWrite = VK_TRUE;

    VkPushConstantRange pcRange{};
    pcRange.size = sizeof(PushConstants);
    pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    conf.pushConstantRanges.push_back(pcRange);

    vk_init::createPipeline(context->device, conf, &pipelineLayout, &pipeline);
    destroyShaders();
}

void DeferredGeometryRenderStage::recordRenderCommands(VkCommandBuffer& cmdBuf, VkBuffer* pVertexBuffer, VkBuffer indexBuffer, uint32_t indexCount)
{
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, pVertexBuffer, offsets);
    vkCmdBindIndexBuffer(cmdBuf, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);
    vkCmdDrawIndexed(cmdBuf, indexCount, 1, 0, 0, 0);
}


void DeferredGeometryRenderStage::recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params)
{
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);



    for (uint32_t i = 0; i < params.geom.size(); i++)
    {
        //std::vector<VkDrawIndexedIndirectCommand> indexBufferData(params.geom[i]->surfs.size());
        //rm->immediateDownloadDeviceBuffer(params.geom[i]->indirectDrawBuffer, indexBufferData);


        vkCmdBindVertexBuffers(cmdBuf, 0, 1, &params.geom[i]->vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(cmdBuf, params.geom[i]->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdPushConstants(cmdBuf, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VkDeviceAddress), &params.geom[i]->instanceBufferAddress);
        vkCmdPushConstants(cmdBuf, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(VkDeviceAddress), sizeof(VkDeviceAddress), &params.geom[i]->materialBufferAddress);
        vkCmdDrawIndexedIndirect(cmdBuf, params.geom[i]->indirectDrawBuffer.buffer, 0, params.geom[i]->surfs.size(), sizeof(VkDrawIndexedIndirectCommand));
        //vkCmdDrawIndexed(cmdBuf, params.geom[i]->numIndices, params.geom[i]->instances.size(), 0, 0, 0);
    }
}

Subpass* DeferredGeometryRenderStage::getSubpass()
{
    return subpass;
}
