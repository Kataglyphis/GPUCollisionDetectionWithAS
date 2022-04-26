#include "DepthPeeling.h"
#include <PipelineToolKit.h>

#define DEPTH_PEELING_HEIGHT 20
#define DEPTH_PEELING_WIDTH 20



DepthPeeling::DepthPeeling()
{
}

DepthPeeling::DepthPeeling(ResourceManager* rm, DescriptorSetManager* descriptorSetManager)
{

    this->descriptorSetManager          = descriptorSetManager;
    this->rm                            = rm;
    this->context                       = rm->context;
    this->descriptorSetDepthPeelingPass = descriptorSetManager->descriptorSetDepthPeelingPass;

    init();
}

void DepthPeeling::reloadShaders()
{
    ppllDepthPeelingRenderStage->destroyPipeline();
    ppllDepthPeelingRenderStage->createPipeline(renderPassPPLLDepthPeeling);
    sortAndFillDepthPeelingRenderStage->destroyPipeline();
    sortAndFillDepthPeelingRenderStage->createPipeline(renderPassSortAndLinkDepthPeeling);
    
}

Buffer* DepthPeeling::get_particle_representation_buffer()
{
    return &particleBuf;
}

Buffer* DepthPeeling::get_counter_buffer()
{
    return &counter_buffer;
}

Buffer* DepthPeeling::get_node_buffer()
{
    return &linked_list;
}

void DepthPeeling::init()
{

	createRenderAttachments();
	createRenderStages();

	createRenderPass();
	createPipelines();

    createFramebuffers();

    createAndUploadBuffersAndImages();

}

void DepthPeeling::recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams drawParams)
{

    uint32_t idx = context->currentSwapchainIndex;

    VkClearColorValue clearColor;
    clearColor.uint32[0] = 0xffffffff;
    VkImageSubresourceRange subresRange = {};

    subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresRange.levelCount = 1;
    subresRange.layerCount = 1;

    vkCmdClearColorImage(cmdBuf, head_pointer[0].image, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);

    // Clear previous geometry pass data
    //vkCmdFillBuffer(cmdBuf, counter_buffer.buffer, 0, sizeof(uint32_t), 0);

    // We need a barrier to make sure all writes are finished before starting to write again
    VkMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 
                                    1, &memoryBarrier, 0, nullptr, 0, nullptr);


    VkClearValue clearValues[2];
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPassPPLLDepthPeeling;
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = { DEPTH_PEELING_WIDTH, DEPTH_PEELING_HEIGHT };
    renderPassBeginInfo.framebuffer = framebuffers[idx];
    renderPassBeginInfo.clearValueCount = 0;
    renderPassBeginInfo.pClearValues = nullptr;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = DEPTH_PEELING_WIDTH;
    viewport.height = DEPTH_PEELING_HEIGHT;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = { DEPTH_PEELING_WIDTH, DEPTH_PEELING_HEIGHT };
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
 
    vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    ppllDepthPeelingRenderStage->recordRenderCommands(cmdBuf, drawParams);

    renderPassBeginInfo.framebuffer = framebuffers_second_pass[idx];
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;
    renderPassBeginInfo.renderPass = renderPassSortAndLinkDepthPeeling;

    vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    sortAndFillDepthPeelingRenderStage->recordRenderCommands(cmdBuf, drawParams);


}

DepthPeeling::~DepthPeeling()
{
}

void DepthPeeling::createRenderAttachments()
{
    this->attachmentInfo.swapchainColorIndex = 0;
    this->attachmentInfo.depthIndex = 5;

    this->attachments.push_back(new ColorAttachment(context));
    this->attachments.push_back(new DepthAttachment(rm));
}

void DepthPeeling::createRenderStages()
{
    ppllDepthPeelingRenderStage = new PPLLRenderStage(rm, descriptorSetDepthPeelingPass, attachmentInfo, 0);
    renderStages.push_back(ppllDepthPeelingRenderStage);

    sortAndFillDepthPeelingRenderStage = new DepthPeelingSortAndFill(rm, descriptorSetDepthPeelingPass, attachmentInfo, 1);
    renderStages.push_back(sortAndFillDepthPeelingRenderStage);

}

void DepthPeeling::createPipelines()
{
    ppllDepthPeelingRenderStage->createPipeline(renderPassPPLLDepthPeeling);
    sortAndFillDepthPeelingRenderStage->createPipeline(renderPassSortAndLinkDepthPeeling);
}

void DepthPeeling::createRenderPass()
{
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 0;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;

    vkCreateRenderPass(context->device, &renderPassInfo, nullptr, &renderPassPPLLDepthPeeling);

    // Need to be filled to create renderpass
    std::vector<VkAttachmentDescription> attachementDescriptions;

    // Fill attachements
    for (uint32_t i = 0; i < attachments.size(); i++)
    {
        attachementDescriptions.push_back(attachments[i]->getAttachmentDescriptor());
    }

    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachementDescriptions.size());

    renderPassInfo.pAttachments = attachementDescriptions.data();

    vkCreateRenderPass(context->device, &renderPassInfo, nullptr, &renderPassSortAndLinkDepthPeeling);

    
}

void DepthPeeling::createFramebuffers()
{

    framebuffers.resize(MAX_FRAME_DRAWS);
    framebuffers_second_pass.resize(MAX_FRAME_DRAWS);

    for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
    {
        VkFramebufferCreateInfo fbufCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fbufCreateInfo.renderPass = renderPassPPLLDepthPeeling;
        fbufCreateInfo.attachmentCount = 0;
        fbufCreateInfo.width = DEPTH_PEELING_WIDTH;
        fbufCreateInfo.height = DEPTH_PEELING_HEIGHT;
        fbufCreateInfo.layers = 1;


        VkResult result = vkCreateFramebuffer(context->device, &fbufCreateInfo, nullptr, &(framebuffers[i]));
        ASSERT_VULKAN(result);
    }

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
        framebufferCreateInfo.renderPass = renderPassSortAndLinkDepthPeeling;
        framebufferCreateInfo.attachmentCount = attachmentViews.size();
        framebufferCreateInfo.pAttachments = attachmentViews.data();
        framebufferCreateInfo.width = DEPTH_PEELING_WIDTH;
        framebufferCreateInfo.height = DEPTH_PEELING_HEIGHT;
        framebufferCreateInfo.layers = 1;


        VkResult result = vkCreateFramebuffer(context->device, &framebufferCreateInfo, nullptr, &(framebuffers_second_pass[i]));
        ASSERT_VULKAN(result);
    }
}



void DepthPeeling::createAndUploadBuffersAndImages()
{
    DepthPeelingConstants constants{};

    constants.projection    = glm::ortho(-1.f,1.f,-1.f,1.f, 0.1f, 2.f);

    //constants.projection    = glm::perspective(glm::radians(60.0f), context->width / (float)context->height, 0.01f, 10.0f);
    glm::vec3 position      = glm::vec3(0,0,2);
    //retrieve the right vector with some world_up
    glm::vec3 front         = { 0.0f, 0.0f, -1.0f};
    glm::vec3 world_up      = { 0.0f, 1.0f, 0.0f };
    glm::vec3 right         = glm::normalize(glm::cross(front, world_up));

    // but this means the up vector must again be calculated with right vector calculated!!!
    glm::vec3 up            = glm::normalize(glm::cross(right, front));
    constants.view          = glm::lookAt(position, position + front, up);

    depthPeelingVector = { constants };
    rm->immediateCreateAndUploadDeviceBuffer<DepthPeelingConstants>(depthPeelingConstantsBuf, 
                                                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                                    depthPeelingVector);

    // for getting all relevant fragments 
    nodes.resize(DEPTH_PEELING_WIDTH * DEPTH_PEELING_HEIGHT * MAX_FRAGMENTS);
    for (int i = 0; i < nodes.size(); i++) {

        nodes[i] =  {   {2*i, 0.f, 0.f, 0.f}//,   // position
                        //0,                      // next
                        //0,                      // depth
                        //0,                      // placeholders
                        //0
                    };

    }

    rm->immediateCreateAndUploadDeviceBuffer<Node>( linked_list,
                                                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT ,
                                                    nodes);

    // storing counters
    counters.emplace_back();
    counters[0] = { 0, DEPTH_PEELING_WIDTH * DEPTH_PEELING_HEIGHT * MAX_FRAGMENTS, 0, 0 };

    rm->immediateCreateAndUploadDeviceBuffer<Counter>( counter_buffer,
                                                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                    counters);

    objectParticles.resize(DEPTH_PEELING_WIDTH * DEPTH_PEELING_HEIGHT * MAX_FRAGMENTS * INSERTION_RATE);

    for (int i = 0; i < objectParticles.size(); i++) {
        glm::vec3 pos = {0.f, -3.f, 0.f};
        glm::vec3 color = { 0.f, 10.f, 0.f };
        glm::vec3 vel = { 0.f, 10.f, 0.f };
        glm::vec3 accel = { 0.f, 10.f, 0.f };

        objectParticles[i] = {  glm::vec4(pos,0.0), 
                                glm::vec4(color,1.0), 
                                glm::vec4(vel,0.0f), 
                                glm::vec4(accel,0.0f) };
    }

    rm->immediateCreateAndUploadDeviceBuffer<Particle>(particleBuf,
                                                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
                                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                    objectParticles);

    VkFormat format = VK_FORMAT_R32_UINT;
    VkImageCreateInfo imgCreateInfo = vk_default::renderTargetImageCreateInfo();
    imgCreateInfo.extent.width = DEPTH_PEELING_WIDTH;
    imgCreateInfo.extent.height = DEPTH_PEELING_HEIGHT;
    imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

    VkImageViewCreateInfo imgViewCreateInfo = vk_default::renderTargetImageViewCreateInfo();
    imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    head_pointer.emplace_back();
    imgCreateInfo.format = format;
    createImage(context->device, context->allocator, imgCreateInfo, head_pointer[0]);
    imgViewCreateInfo.format = format;
    imgViewCreateInfo.image = head_pointer[0].image;
    createImageView(context->device, imgViewCreateInfo, head_pointer[0].view);

    rm->immediateImageLayoutTransition(head_pointer[0], VK_IMAGE_LAYOUT_GENERAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);

    vkDeviceWaitIdle(context->device);

    descriptorSetManager->addDescriptorUpdate(linked_list, descriptorSetManager->resourceBindings.depthPeelingLinkedList, true);
    descriptorSetManager->addDescriptorUpdate(counter_buffer, descriptorSetManager->resourceBindings.depthPeelingCounter, true);
    descriptorSetManager->addDescriptorUpdate(particleBuf, descriptorSetManager->resourceBindings.depthPeelingParticleBuffer, true);
    descriptorSetManager->addDescriptorUpdate(depthPeelingConstantsBuf, descriptorSetManager->resourceBindings.depthPeelingConstants, true);
    
    descriptorSetManager->addDescriptorUpdate(head_pointer, descriptorSetManager->resourceBindings.depthPeelingHeadPointer, true);

}
