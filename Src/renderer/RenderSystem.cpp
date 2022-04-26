#include "RenderSystem.h"

RenderSystem::RenderSystem(ResourceManager* rm, Camera* camera, DescriptorSetManager* descriptorSetManager, Scene* scene)
{
	this->rm = rm;
    this->scene = scene;
    this->camera = camera;
    context = rm->context;
    this->descriptorSetManager = descriptorSetManager;
    this->descriptorSetGeometryPass = descriptorSetManager->descriptorSetGeometryPass;
    this->descriptorSetShadingPass = descriptorSetManager->descriptorSetShadingPass;
    this->descriptorSetParticlePass = descriptorSetManager->descriptorSetParticlePass;
    this->descriptorSetPathTracing = descriptorSetManager->descriptorSetPathTracing;
    this->descriptorSetDepthPeelingPass = descriptorSetManager->descriptorSetDepthPeelingPass;

	init();
}

RenderSystem::~RenderSystem()
{
}



void RenderSystem::init()
{
	this->deferredPipeline = new DeferredPipeline(rm, descriptorSetGeometryPass, descriptorSetShadingPass, descriptorSetParticlePass);

#ifdef PATH_TRACING
    this->ptPipeline = new PathTraceRenderStage(rm, descriptorSetManager);
#endif

    this->depthPeeling = new DepthPeeling(rm, descriptorSetManager);

    addStaticDescriptorWrites();
    createSynchronizationObjects();
}

void RenderSystem::addStaticDescriptorWrites()
{

    auto addInputAttachement = [&](uint32_t binding, RenderAttachment* attachment)
    {
        VkDescriptorImageInfo info = {};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageView = attachment->getImageView(0);
        info.sampler = VK_NULL_HANDLE;

        DescriptorSetUpdate update = {};
        update.dstBinding = binding;
        update.descriptorCount = 1;
        update.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        update.imageInfo = { info };

        for (uint32_t i = 0; i < MAX_FRAME_DRAWS; i++)
        {
            descriptorSetShadingPass->addUpdate(update, i);
        }
    };

    

    std::vector<RenderAttachment*> attachments      = this->deferredPipeline->attachments;
    RenderAttachmentInfo attachmentInfo             = this->deferredPipeline->attachmentInfo;

    addInputAttachement(1, attachments[attachmentInfo.albedoIndex]); // albedo
    addInputAttachement(2, attachments[attachmentInfo.normalIndex]); // normals
    addInputAttachement(3, attachments[attachmentInfo.materialIndex]); // rougness + metalicity
    addInputAttachement(4, attachments[attachmentInfo.positionIndex]); // position

}

void RenderSystem::render()
{
	update();
	draw();
}

void RenderSystem::reloadShaders()
{
    vkDeviceWaitIdle(context->device);
    this->deferredPipeline->reloadShaders();
#ifdef PATH_TRACING
    if (g_enable_path_tracing.val.v_bool)
    {
        ptPipeline->reloadShaders();
    }
#endif // PATH_TRACEING

    this->depthPeeling->reloadShaders();

}

void RenderSystem::createDrawCmdBuf()
{
    DrawParams params{};
    params.pVertexBufferScene = &scene->geom[0]->vertexBuffer.buffer;
    params.indexBufferScene =    scene->geom[0]->indexBuffer.buffer;
    params.indexCountScene =     scene->geom[0]->numIndices;
    params.geom = scene->geom;
    params.inst = scene->inst;

    params.pVertexBufferParticles =  &context->storage_buffer_read_results[context->currentSwapchainIndex].buffer;
    params.vertexCountParticles = context->numParticlesX * context->numParticlesY * context->numParticlesZ;

    params.pVertexBufferSceneParticles = &depthPeeling->get_particle_representation_buffer()->buffer;
    //params.vertexCountSceneParticles = context->width * context->height * MAX_FRAGMENTS * INSERTION_RATE;
    params.vertexCountSceneParticles = 100 * MAX_FRAGMENTS * INSERTION_RATE;

    // DEBUG
    //std::vector<Particle> particles;
    //particles.resize(context->width * context->height * MAX_FRAGMENTS);
    //rm->immediateDownloadDeviceBuffer(*depthPeeling->get_particle_representation_buffer(), particles);

    //// storage for all object particles 
    //std::vector<Counter> counters_copy;
    //counters_copy.resize(1);
    //rm->immediateDownloadDeviceBuffer(*depthPeeling->get_counter_buffer(), counters_copy);


    //std::vector<Node> node_copy;
    //node_copy.resize(context->width* context->height * MAX_FRAGMENTS);
    //rm->immediateDownloadDeviceBuffer(*depthPeeling->get_node_buffer(), node_copy);

    drawCmdBuf = rm->startSingleTimeCmdBuffer();
    setDebugMarker(rm->context->device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)drawCmdBuf, "Render Cmd Buf");
    // Path tracing
#ifdef PATH_TRACING
    if (g_enable_path_tracing.val.v_bool)
    {
        ptPipeline->recordRenderCommands(drawCmdBuf, params);
    }
#endif

    // Deferred Shading
    deferredPipeline->recordRenderCommands(drawCmdBuf, params);

    // depth peeling 
    //if (!init_depth_peeling) {
    //    init_depth_peeling = true;
    //    depthPeeling->recordRenderCommands(drawCmdBuf, params);
    //}

    VkResult result = vkEndCommandBuffer(drawCmdBuf);
    ASSERT_VULKAN(result);



}

void RenderSystem::update()
{
	
    
	descriptorSetGeometryPass->processUpdates(context->device, context->currentSwapchainIndex);
	descriptorSetShadingPass->processUpdates(context->device, context->currentSwapchainIndex);
	descriptorSetParticlePass->processUpdates(context->device, context->currentSwapchainIndex);
	descriptorSetPathTracing->processUpdates(context->device, context->currentSwapchainIndex);
    descriptorSetDepthPeelingPass->processUpdates(context->device, context->currentSwapchainIndex);

	createDrawCmdBuf();

}

void RenderSystem::createSynchronizationObjects()
{
    context->imageAvailableSemaphores.resize(MAX_FRAME_DRAWS);
    context->renderFinishedSemaphores.resize(MAX_FRAME_DRAWS);
    context->physicsFinishedSemaphores.resize(MAX_FRAME_DRAWS);
    context->uploadFinishedSemaphores.resize(MAX_FRAME_DRAWS);
    context->asFinished.resize(MAX_FRAME_DRAWS);

    context->inFlightFences.resize(MAX_FRAME_DRAWS);
    context->imagesInFlight.resize(MAX_FRAME_DRAWS);
    for (int x = 0; x < MAX_FRAME_DRAWS; x++) {
        context->imagesInFlight[x] = VK_NULL_HANDLE;
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int x = 0; x < MAX_FRAME_DRAWS; x++) {
        VkResult result = vkCreateSemaphore(context->device, &semaphoreCreateInfo, NULL, &(context->imageAvailableSemaphores[x]));
        ASSERT_VULKAN(result);
        result = vkCreateSemaphore(context->device, &semaphoreCreateInfo, NULL, &(context->renderFinishedSemaphores[x]));
        ASSERT_VULKAN(result);
        result = vkCreateSemaphore(context->device, &semaphoreCreateInfo, NULL, &(context->physicsFinishedSemaphores[x]));
        ASSERT_VULKAN(result);
        result = vkCreateSemaphore(context->device, &semaphoreCreateInfo, NULL, &(context->uploadFinishedSemaphores[x]));
        ASSERT_VULKAN(result);
        result = vkCreateSemaphore(context->device, &semaphoreCreateInfo, NULL, &(context->asFinished[x]));
        ASSERT_VULKAN(result);
        result = vkCreateFence(context->device, &fenceCreateInfo, NULL, &(context->inFlightFences[x]));
        ASSERT_VULKAN(result);
    }
}


void RenderSystem::draw()
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(context->device, context->swapchain, UINT64_MAX, context->imageAvailableSemaphores[context->currentSwapchainIndex], VK_NULL_HANDLE, &imageIndex);
    ASSERT_VULKAN(result);

    
    context->imagesInFlight[imageIndex] = context->inFlightFences[context->currentSwapchainIndex];


    std::vector<VkSemaphore>            waitSemaphores = {
        context->imageAvailableSemaphores[context->currentSwapchainIndex],
        //context->asFinished[context->currentSwapchainIndex],
        context->renderComputeSync[context->currentSwapchainIndex],
        context->physicsFinishedSemaphores[context->currentSwapchainIndex],
        context->uploadFinishedSemaphores[context->currentSwapchainIndex]
    };
    std::vector<VkPipelineStageFlags>   waitStages = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        //VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT
    };
    std::vector < VkSemaphore>          signalSemaphores = { context->renderFinishedSemaphores[context->currentSwapchainIndex] };

    vkResetFences(context->device, 1, &context->inFlightFences[context->currentSwapchainIndex]);

    SubmitSynchronizationInfo syncInfo = {};
    syncInfo.waitSemaphoreCount = waitSemaphores.size();
    syncInfo.pWaitSemaphore = waitSemaphores.data();
    syncInfo.pWaitDstStageMask = waitStages.data();
    syncInfo.signalSemaphoreCount = signalSemaphores.size();
    syncInfo.pSignalSemaphore = signalSemaphores.data();
    syncInfo.signalFence = context->inFlightFences[context->currentSwapchainIndex];

    rm->cmdBufSubmitSynchronized(&drawCmdBuf, 1, syncInfo, true);
    //vkDeviceWaitIdle(context->device);
    rm->present(signalSemaphores, imageIndex);

    context->currentSwapchainIndex = (context->currentSwapchainIndex + 1) % MAX_FRAME_DRAWS;
    context->totalFrameIndex++;
}
