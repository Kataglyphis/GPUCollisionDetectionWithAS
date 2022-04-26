#include "ClassicPhysics.h"

ClassicPhysics::ClassicPhysics(ResourceManager* rm, DescriptorSetManager* descriptorSetManager)
{
	this->rm = rm;
	this->context = rm->context;
	this->descManager = descriptorSetManager;
	createBuffers();
	this->modelMatrixComputeStage = new ModelMatrixComputeStage(context, descManager->descriptorSetRtPhysicsIntegration);
	this->voxelGenStage = new GlobalVoxelGridGenStage(context, descManager->descriptorSetClassicPhysics);
	this->voxelIntegrationStage = new VoxelIntegrationStage(context, descManager->descriptorSetClassicPhysics);
	setUpTimeQuerys();
}

ClassicPhysics::~ClassicPhysics()
{
}

void ClassicPhysics::setUpTimeQuerys()
{
	VkQueryPoolCreateInfo queryPoolCI{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	queryPoolCI.queryType = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolCI.queryCount = 2;
	VkResult result = vkCreateQueryPool(context->device, &queryPoolCI, nullptr, &queryPool);
	ASSERT_VULKAN(result);
}

void ClassicPhysics::createBuffers()
{
	uint32_t gridSize = globalVoxelGridExtent * globalVoxelGridExtent * globalVoxelGridExtent;
	void* data = calloc(gridSize, sizeof(VoxelGridEntry));
	rm->immediatCreateAndUploadDataToDeviceBuffer(globalVoxelGridBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		data, gridSize * sizeof(VoxelGridEntry));

	createBufferWithAllocator(context->device, context->allocator, MAX_INSTANCES * sizeof(InstanceShaderData),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY, instanceStorageBuffer.buffer, instanceStorageBuffer.allocation);


	descManager->addDescriptorUpdate(globalVoxelGridBuffer, descManager->resourceBindings.globalVoxelGrid, true);
	descManager->addDescriptorUpdate(instanceStorageBuffer, descManager->resourceBindings.instanceData, true);
	//VkDescriptorBufferInfo info = {};
	//info.buffer = instanceStorageBuffer.buffer;
	//info.offset = 0;
	//info.range = VK_WHOLE_SIZE;
	//
	//DescriptorSetUpdate update = {};
	//update.dstBinding = 1;
	//update.descriptorCount = 1;
	//update.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	//update.bufferInfo = { info };
	//
	//for (uint32_t i = 0; i < MAX_FRAME_DRAWS; i++)
	//{
	//	descManager->descriptorSetClassicPhysics->addUpdate(update, i);
	//}
}

void ClassicPhysics::runSimulation(Scene* scene)
{
	descManager->addDescriptorUpdate(instanceStorageBuffer, descManager->resourceBindings.instanceData, true);
	descManager->descriptorSetClassicPhysics->processUpdates(context->device, context->currentSwapchainIndex);
	descManager->descriptorSetRtPhysicsIntegration->processUpdates(context->device, context->currentSwapchainIndex);

	if (context->currentSwapchainIndex == 0)
	{
		VkResult result = vkGetQueryPoolResults(
			context->device,
			queryPool,
			0,
			2,
			2 * sizeof(uint64_t),
			queryResults,
			static_cast<VkDeviceSize>(sizeof(uint64_t)),
			VK_QUERY_RESULT_64_BIT);

		if (result != VK_NOT_READY)
		{
			uint64_t delta_t_ns = (queryResults[1] - queryResults[0]) * context->physicalDeviceProps.limits.timestampPeriod;
			g_physics_timings.val.v_float = static_cast<float>(delta_t_ns) / 1000000.f;
		}
	}

	createSimulationCommandBuffer(scene);
	SubmitSynchronizationInfo syncInfo{};
	std::vector<VkPipelineStageFlags> stages = { VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT };
	std::vector <VkSemaphore> waitSemaphores = { context->asFinished[context->currentSwapchainIndex] , context->uploadFinishedSemaphores[context->currentSwapchainIndex] };
	syncInfo.waitSemaphoreCount = waitSemaphores.size();
	syncInfo.pWaitSemaphore = waitSemaphores.data();
	syncInfo.pWaitDstStageMask = stages.data();
	syncInfo.signalSemaphoreCount = 1;
	syncInfo.pSignalSemaphore = &context->physicsFinishedSemaphores[context->currentSwapchainIndex];
	rm->cmdBufSubmitSynchronized(&simulationCmdBuf, 1, syncInfo, true);
}

void ClassicPhysics::createSimulationCommandBuffer(Scene* scene)
{

	DrawGeom* geom = scene->geom[1];
	uint32_t instanceCount = geom->instances.size();
	simulationCmdBuf = rm->startSingleTimeCmdBuffer();
	
	if (context->currentSwapchainIndex == 0)
	{
		vkCmdResetQueryPool(simulationCmdBuf, queryPool, 0, 2);
		vkCmdWriteTimestamp(simulationCmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 0);
	}

	VkBufferCopy copyInfo{};
	copyInfo.size = instanceCount * sizeof(InstanceShaderData);
	vkCmdCopyBuffer(simulationCmdBuf, geom->instanceBuffer.buffer, instanceStorageBuffer.buffer, 1, &copyInfo);

	// Barrier ---------------------------------------
	VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(simulationCmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0, 1, &barrier, 0, nullptr, 0, nullptr);
	
	
	this->voxelGenStage->recordRenderCommands(simulationCmdBuf, { geom });

	// Barrier ---------------------------------------
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(simulationCmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0, 1, &barrier, 0, nullptr, 0, nullptr);

	this->voxelIntegrationStage->recordRenderCommands(simulationCmdBuf, { geom });

	// Barrier ---------------------------------------
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(simulationCmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0, 1, &barrier, 0, nullptr, 0, nullptr);

	this->modelMatrixComputeStage->recordRenderCommands(simulationCmdBuf, { geom });


	// Barrier ---------------------------------------
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	vkCmdPipelineBarrier(simulationCmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 1, &barrier, 0, nullptr, 0, nullptr);


	vkCmdCopyBuffer(simulationCmdBuf, instanceStorageBuffer.buffer, geom->instanceBuffer.buffer, 1, &copyInfo);

	if (context->currentSwapchainIndex == 0)vkCmdWriteTimestamp(simulationCmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 1);

	vkEndCommandBuffer(simulationCmdBuf);
}

void ClassicPhysics::reloadShaders()
{
	vkDeviceWaitIdle(context->device);
	this->voxelGenStage->destroyPipeline();
	this->voxelIntegrationStage->destroyPipeline();
	this->modelMatrixComputeStage->destroyPipeline();
	this->voxelGenStage->createPipeline();
	this->voxelIntegrationStage->createPipeline();
	this->modelMatrixComputeStage->createPipeline();
}
