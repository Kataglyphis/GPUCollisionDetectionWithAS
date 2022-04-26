#include "DeferredPipeline.h"
#include "DeferredGeometrySubpass.h"
#include "DeferredShadingSubpass.h"
#include "PipelineToolKit.h"
#include "Vertex.h"
#include "DeferredParticleSubpass.h"


DeferredPipeline::DeferredPipeline()
{
}

DeferredPipeline::DeferredPipeline(ResourceManager* rm, DescriptorSet* descriptorSetDeferredGeometry, DescriptorSet* descriptorSetDeferredShading, DescriptorSet* descriptorSetDeferredParticle)
{
    this->descSetGeometry = descriptorSetDeferredGeometry;
    this->descSetShading  = descriptorSetDeferredShading;
    this->descSetParticle = descriptorSetDeferredParticle;
    this->rm = rm;
    context = rm->context;
    init();
}

void DeferredPipeline::reloadShaders()
{
    //this->geometryStage->destroyPipeline();
    //this->shadingStage->destroyPipeline();
    //this->particleStage->destroyPipeline();
    //
    //this->geometryStage->createPipeline(renderPassDeferred);
    //this->shadingStage->createPipeline(renderPassDeferred);
    //this->particleStage->createPipeline(renderPassDeferred);
    for (uint32_t i = 0; i < renderStages.size(); i++)
    {
        renderStages[i]->destroyPipeline();
        renderStages[i]->createPipeline(renderPassDeferred);
    }
}


void DeferredPipeline::createRenderAttachments()
{
    this->attachmentInfo.swapchainColorIndex = 0;
    this->attachmentInfo.albedoIndex = 1;
    this->attachmentInfo.normalIndex = 2;
    this->attachmentInfo.materialIndex = 3;
    this->attachmentInfo.positionIndex = 4;
    this->attachmentInfo.depthIndex = 5;

    this->attachments.push_back(new ColorAttachment(context));
    this->attachments.push_back(new InputAttachment(context, VK_FORMAT_R8G8B8A8_SRGB));
    this->attachments.push_back(new InputAttachment(context, VK_FORMAT_A2R10G10B10_UNORM_PACK32));
    this->attachments.push_back(new InputAttachment(context, VK_FORMAT_R8G8_SNORM));
    this->attachments.push_back(new InputAttachment(context, VK_FORMAT_R16G16B16A16_SFLOAT));
    this->attachments.push_back(new DepthAttachment(rm));
}

void DeferredPipeline::createRenderStages()
{
    geometryStage = new DeferredGeometryRenderStage(rm, descSetGeometry, attachmentInfo, 0);
    renderStages.push_back(geometryStage);

//#ifdef 0
//    particleStage = new ParticleRenderStage(context, descSetParticle, attachmentInfo, 1);
//    imGuiStage = new ImGuiRenderStage(rm, descSetParticle, attachmentInfo, 2);
//    renderStages.push_back(particleStage);
//    renderStages.push_back(imGuiStage);
//#else
    shadingStage = new DeferredShadingRenderStage(context, descSetShading, attachmentInfo, 1);
    particleStage = new ParticleRenderStage(context, descSetParticle, attachmentInfo, 2);
    imGuiStage = new ImGuiRenderStage(rm, descSetParticle, attachmentInfo, 3);

    renderStages.push_back(shadingStage);
    renderStages.push_back(particleStage);
    renderStages.push_back(imGuiStage);
//#endif
    
}

void DeferredPipeline::createPipelines()
{
    //geometryStage->createPipeline(renderPassDeferred);
    //shadingStage->createPipeline(renderPassDeferred);
    //particleStage->createPipeline(renderPassDeferred);
    for (uint32_t i = 0; i < renderStages.size(); i++)
    {
        renderStages[i]->createPipeline(renderPassDeferred);
    }
}

void DeferredPipeline::init()
{
    createRenderAttachments();
    createRenderStages();

    createRenderPass();
    createPipelines();

    createFramebuffers();
}

void DeferredPipeline::recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams drawParams)
{

    std::vector<VkClearValue> clearValues;
    for (uint32_t i = 0; i < attachments.size(); i++)
    {
        clearValues.push_back(attachments[i]->getClearValue());
    }

    uint32_t idx = context->currentSwapchainIndex;

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPassDeferred;
    renderPassBeginInfo.framebuffer = framebuffers[idx];
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = { context->width, context->height };
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = context->width;
    viewport.height = context->height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = { context->width, context->height };
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    for (uint32_t i = 0; i < renderStages.size(); i++)
    {
        renderStages[i]->recordRenderCommands(cmdBuf, drawParams);
    }

    //geometryStage->recordRenderCommands(cmdBuf, drawParams.pVertexBufferScene, drawParams.indexBufferScene, drawParams.indexCountScene);
    //shadingStage->recordRenderCommands(cmdBuf);
    //ParticleStage->recordRenderCommands(cmdBuf, drawParams.pVertexBufferParticles, drawParams.vertexCountParticles);

    vkCmdEndRenderPass(cmdBuf);
}



DeferredPipeline::~DeferredPipeline()
{
}


void DeferredPipeline::createFramebuffers() {

    framebuffers.resize(MAX_FRAME_DRAWS);

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
    {
        std::vector<VkImageView> attachmentViews;
        for (uint32_t j = 0; j < attachments.size(); j++)
        {
            attachmentViews.push_back(attachments[j]->getImageView(i));
        }

        VkFramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.pNext = nullptr;
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.renderPass = renderPassDeferred;
        framebufferCreateInfo.attachmentCount = attachmentViews.size();
        framebufferCreateInfo.pAttachments = attachmentViews.data();
        framebufferCreateInfo.width = context->width;
        framebufferCreateInfo.height = context->height;
        framebufferCreateInfo.layers = 1;


        VkResult result = vkCreateFramebuffer(context->device, &framebufferCreateInfo, nullptr, &(framebuffers[i]));
        ASSERT_VULKAN(result);
    }
}

void DeferredPipeline::createRenderPass()
{
	// Need to be filled to create renderpass
	std::vector<VkAttachmentDescription> attachementDescriptions;

	// Fill attachements
    for (uint32_t i = 0; i < attachments.size(); i++)
    {
        attachementDescriptions.push_back(attachments[i]->getAttachmentDescriptor());
    }

	// Create subpasses
    //std::vector<Subpass*> subpasses =
    //{
    //    geometryStage->getSubpass(),
    //    shadingStage->getSubpass(),
    //    particleStage->getSubpass()
	//};
    std::vector<Subpass*> subpasses;
    for (uint32_t i = 0; i < renderStages.size(); i++)
    {
        subpasses.push_back(renderStages[i]->getSubpass());
    }

	// Create Renderpass
	vk_init::createRenderPass(context->device, attachementDescriptions, subpasses, renderPassDeferred);
}




VkVertexInputBindingDescription Particle::getBindingDescription() {
    VkVertexInputBindingDescription vertexInputBindingDescription = {};
    vertexInputBindingDescription.binding = 0;
    vertexInputBindingDescription.stride = sizeof(Particle);
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return vertexInputBindingDescription;
}
std::vector<VkVertexInputAttributeDescription> Particle::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
    std::vector<VkFormat> formats = {
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32B32_SFLOAT,
    };
    std::vector<uint32_t> offsets = {
        offsetof(Particle, position),
        offsetof(Particle, color),
        offsetof(Particle, velocity),
        offsetof(Particle, acceleration)
    };
    for (size_t i = 0; i < formats.size(); i++)
    {
        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.location = i;
        attributeDescription.format = formats[i];
        attributeDescription.offset = offsets[i];
        vertexInputAttributeDescriptions.push_back(attributeDescription);
    }
    return vertexInputAttributeDescriptions;
}
