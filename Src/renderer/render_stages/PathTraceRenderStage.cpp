#include "PathTraceRenderStage.h"
#include "RtUtility.h"
#include "DrawGeom.h"
#include "DescriptorSetManager.h"

PathTraceRenderStage::PathTraceRenderStage(ResourceManager* rm, DescriptorSetManager* descriptorSetManager)
{
	this->rm = rm;
	this->context = rm->context;
	this->descriptorSet = descriptorSetManager->descriptorSetPathTracing;
	this->descriptorSetManager = descriptorSetManager;

	createPipeline();
	createOffscreenImage();

}

void PathTraceRenderStage::reloadShaders()
{
	destroyPipeline();
	createPipeline();
}

PathTraceRenderStage::~PathTraceRenderStage()
{
}

void PathTraceRenderStage::destroyPipeline()
{
	vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
	vkDestroyPipeline(context->device, pipeline, nullptr);
}

void PathTraceRenderStage::createPipeline()
{

	VK_LOAD(vkCreateRayTracingPipelinesKHR);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups(5);

	auto addStage = [&](std::string shader, VkShaderModule& shaderModule,
		VkShaderStageFlagBits flags, std::vector<VkRayTracingShaderGroupCreateInfoKHR*> groups)
	{
		for (uint32_t i = 0; i < groups.size(); i++)
		{
			uint32_t stageIndex = shaderStages.size();
			switch (flags)
			{
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
				groups[i]->generalShader = stageIndex;
				break;
			case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
				groups[i]->anyHitShader = stageIndex;
				break;
			case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
				groups[i]->closestHitShader = stageIndex;
				break;
			case VK_SHADER_STAGE_MISS_BIT_KHR:
				groups[i]->generalShader = stageIndex;
				break;
			default:
				break;
			}
		}
		compileShader(context->device, shader, &shaderModule);
		VkPipelineShaderStageCreateInfo stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		stage.stage = flags;
		stage.module = shaderModule;
		stage.pName = "main";
		shaderStages.push_back(stage);
	};

	// Get empty groups
	VkRayTracingShaderGroupCreateInfoKHR emptyGroup{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
	emptyGroup.anyHitShader			= VK_SHADER_UNUSED_KHR;
	emptyGroup.closestHitShader		= VK_SHADER_UNUSED_KHR;
	emptyGroup.generalShader		= VK_SHADER_UNUSED_KHR;
	emptyGroup.intersectionShader	= VK_SHADER_UNUSED_KHR;
	emptyGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;

	VkRayTracingShaderGroupCreateInfoKHR genGroup{ emptyGroup };
	VkRayTracingShaderGroupCreateInfoKHR missGroup{ emptyGroup };
	VkRayTracingShaderGroupCreateInfoKHR shadowMissGroup{ emptyGroup };
	VkRayTracingShaderGroupCreateInfoKHR opaqueHitGroup{ emptyGroup };
	opaqueHitGroup.type			= VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	VkRayTracingShaderGroupCreateInfoKHR transparentHitGroup{ emptyGroup };
	transparentHitGroup.type	= VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;


	addStage(shaders.generation,			shaderModules.generation,			VK_SHADER_STAGE_RAYGEN_BIT_KHR,		{ &genGroup });
	addStage(shaders.miss,					shaderModules.miss,					VK_SHADER_STAGE_MISS_BIT_KHR,		{ &missGroup });
	addStage(shaders.shadowMiss,			shaderModules.shadowMiss,			VK_SHADER_STAGE_MISS_BIT_KHR,		{ &shadowMissGroup });
	addStage(shaders.closest_hit,			shaderModules.closest_hit,			VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,{ &opaqueHitGroup , &transparentHitGroup });
	addStage(shaders.any_hit_transparent,	shaderModules.any_hit_transparent,	VK_SHADER_STAGE_ANY_HIT_BIT_KHR,	{ &opaqueHitGroup , &transparentHitGroup });
	//addStage(shaders.any_hit_opaque,		shaderModules.any_hit_opaque,		VK_SHADER_STAGE_ANY_HIT_BIT_KHR,	{ &opaqueHitGroup });

	groups[shaderGroupIndices.generation]		= genGroup;
	groups[shaderGroupIndices.miss]				= missGroup;
	groups[shaderGroupIndices.miss_shadow]		= shadowMissGroup;
	groups[shaderGroupIndices.hit_opaque]		= opaqueHitGroup;
	groups[shaderGroupIndices.hit_transparent]	= transparentHitGroup;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSet->layout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

	VkResult result = vkCreatePipelineLayout(context->device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
	ASSERT_VULKAN(result);


	VkPipelineLibraryCreateInfoKHR pipelineLibraryCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR };
	pipelineLibraryCreateInfo.pNext = NULL;
	pipelineLibraryCreateInfo.libraryCount = 0;
	pipelineLibraryCreateInfo.pLibraries = NULL;

	VkRayTracingPipelineCreateInfoKHR rayPipelineCreateInfo{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR };
	rayPipelineCreateInfo.pNext = NULL;
	rayPipelineCreateInfo.flags = 0;
	rayPipelineCreateInfo.stageCount = shaderStages.size();
	rayPipelineCreateInfo.pStages = shaderStages.data();
	rayPipelineCreateInfo.groupCount = groups.size();
	rayPipelineCreateInfo.pGroups = groups.data();
	rayPipelineCreateInfo.maxPipelineRayRecursionDepth = ray_max_recursion_level;
	rayPipelineCreateInfo.pLibraryInfo = &pipelineLibraryCreateInfo;
	rayPipelineCreateInfo.pLibraryInterface = NULL;
	rayPipelineCreateInfo.layout = pipelineLayout;
	rayPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	rayPipelineCreateInfo.basePipelineIndex = -1;

	result = pvkCreateRayTracingPipelinesKHR(context->device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayPipelineCreateInfo, NULL, &pipeline);
	ASSERT_VULKAN(result);
	
	vkDestroyShaderModule(context->device, shaderModules.any_hit_transparent, NULL);
	vkDestroyShaderModule(context->device, shaderModules.closest_hit, NULL);
	vkDestroyShaderModule(context->device, shaderModules.generation, NULL);
	vkDestroyShaderModule(context->device, shaderModules.miss, NULL);
	vkDestroyShaderModule(context->device, shaderModules.shadowMiss, NULL);
	//vkDestroyShaderModule(context->device, shaderModules.any_hit_opaque, NULL);
}


ShaderGroupHandle PathTraceRenderStage::getShaderGroupHandle()
{
	VK_LOAD(vkGetRayTracingShaderGroupHandlesKHR);

	ShaderGroupHandle groupHandle{};
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
	VkPhysicalDeviceProperties2 properties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
	properties.pNext = &rayTracingProperties;
	vkGetPhysicalDeviceProperties2(context->physDevice, &properties);
	uint32_t groupSizeAligned = alignUp(rayTracingProperties.shaderGroupHandleSize, rayTracingProperties.shaderGroupBaseAlignment);

	VkDeviceSize groupHandleStorageSize = groupSizeAligned * (groupCount); //civ
	groupHandle.handles = std::vector<uint8_t>(groupHandleStorageSize);
	groupHandle.handleSize = rayTracingProperties.shaderGroupHandleSize;
	groupHandle.handleSizeAligned = groupSizeAligned;
	groupHandle.baseAlignement = rayTracingProperties.shaderGroupBaseAlignment;
	VkResult result = pvkGetRayTracingShaderGroupHandlesKHR(context->device, pipeline, 0, groupCount, groupHandleStorageSize, groupHandle.handles.data());
	ASSERT_VULKAN(result);
	return groupHandle;
}

void PathTraceRenderStage::buildHitSBT(VkCommandBuffer cmdBuf, std::vector<DrawInst*> instances)
{
	VK_LOAD(vkGetRayTracingShaderGroupHandlesKHR);

	ShaderGroupHandle groupHandle = getShaderGroupHandle();
	std::vector<uint32_t> sbtInstanceOffsets = vk_rt::getInstanceShaderBindingTableRecordOffsets(instances, rayTypes);

	// Calc size
	VkDeviceSize shaderBindingTableSize =
		(sbtInstanceOffsets.back() + instances.back()->geometry->surfs.size() * rayTypes)
		* groupHandle.handleSizeAligned;

	// Create sbt buffer
	Buffer stagingBuffer{};
	createBufferWithAllocator(context->device, context->allocator,
		shaderBindingTableSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer.buffer, stagingBuffer.allocation);

	//createBufferNoAllocator(context->device, context->physDevice,
	//	shaderBindingTableSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	//	sbts.hit.buf.buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sbts.hit.buf.memory);

	rm->addToDeleteQueue(&stagingBuffer, 1);

	// Fill sbt buffer
	void* pHitGroupOpaqueShaderHandle = groupHandle.handles.data() + groupHandle.handleSize * shaderGroupIndices.hit_opaque;
	void* pHitGroupTransparentShaderHandle = groupHandle.handles.data() + groupHandle.handleSize * shaderGroupIndices.hit_transparent;
	void* data;
	uint32_t offset = 0;
	DrawGeom* lastGeometry = nullptr;
	context->allocator->mapMemory(stagingBuffer.allocation, &data);
	//vkMapMemory(context->device, sbts.hit.buf.memory, 0, shaderBindingTableSize, 0, &data);
	for (uint32_t i = 0; i < instances.size(); i++)
	{
		if (instances[i]->geometry != lastGeometry)
		{
			for (uint32_t j = 0; j < instances[i]->geometry->surfs.size(); j++)
			{
				DrawSurf* surf = &instances[i]->geometry->surfs[j];

				offset = (sbtInstanceOffsets[i] + j * rayTypes) * groupHandle.handleSizeAligned;
				switch (surf->opacity)
				{
				case DrawSurf::Opacity::OPAQUE:
					memcpy((char*)data + offset, pHitGroupOpaqueShaderHandle, groupHandle.handleSize);
					break;
				case DrawSurf::Opacity::CUT_OUT:
					memcpy((char*)data + offset, pHitGroupTransparentShaderHandle, groupHandle.handleSize);
					break;
				default:
					break;
				}
			}

			lastGeometry = instances[i]->geometry;
		}
	}



	//vkUnmapMemory(context->device, sbts.hit.buf.memory);
	context->allocator->unmapMemory(stagingBuffer.allocation);

	// Copy sbt buffer
	createBufferWithAllocator(context->device, context->allocator,
		shaderBindingTableSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY, sbts.hit.buf.buffer, sbts.hit.buf.allocation);
	rm->addToDeleteQueue(&sbts.hit.buf, 1);
	recordCopyBuffer(cmdBuf, stagingBuffer.buffer, sbts.hit.buf.buffer, shaderBindingTableSize);

	// Set region
	sbts.hit.region.deviceAddress = getBufferAddressKHR(context, sbts.hit.buf.buffer);
	sbts.hit.region.stride = groupHandle.handleSizeAligned;
	sbts.hit.region.size = shaderBindingTableSize;
}

void PathTraceRenderStage::buildGenerationSBT(VkCommandBuffer cmdBuf)
{
	VK_LOAD(vkGetRayTracingShaderGroupHandlesKHR);

	ShaderGroupHandle groupHandle = getShaderGroupHandle();

	// Calc size
	VkDeviceSize shaderBindingTableSize = groupHandle.handleSizeAligned;

	// Create sbt buffer
	Buffer stagingBuffer{};
	createBufferWithAllocator(context->device, context->allocator,
		shaderBindingTableSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer.buffer, stagingBuffer.allocation);

	//createBufferNoAllocator(context->device, context->physDevice,
	//	shaderBindingTableSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	//	sbts.generation.buf.buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sbts.generation.buf.memory);

	rm->addToDeleteQueue(&stagingBuffer, 1);

	// Fill sbt buffer
	void* pGenerationGroupShaderHandle = groupHandle.handles.data() + groupHandle.handleSize * shaderGroupIndices.generation;
	void* data;
	context->allocator->mapMemory(stagingBuffer.allocation, &data);
	//vkMapMemory(context->device, sbts.generation.buf.memory, 0, shaderBindingTableSize, 0, &data);
	memcpy(data, pGenerationGroupShaderHandle, groupHandle.handleSize);
	//vkUnmapMemory(context->device, sbts.generation.buf.memory);
	context->allocator->unmapMemory(stagingBuffer.allocation);

	// Copy sbt buffer
	createBufferWithAllocator(context->device, context->allocator,
		shaderBindingTableSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY, sbts.generation.buf.buffer, sbts.generation.buf.allocation);
	rm->addToDeleteQueue(&sbts.generation.buf, 1);
	recordCopyBuffer(cmdBuf, stagingBuffer.buffer, sbts.generation.buf.buffer, shaderBindingTableSize);


	// Set region
	sbts.generation.region.deviceAddress = getBufferAddressKHR(context, sbts.generation.buf.buffer);
	sbts.generation.region.stride = groupHandle.handleSizeAligned;
	sbts.generation.region.size = shaderBindingTableSize;
}

void PathTraceRenderStage::buildMissSBT(VkCommandBuffer cmdBuf)
{
	VK_LOAD(vkGetRayTracingShaderGroupHandlesKHR);

	ShaderGroupHandle groupHandle = getShaderGroupHandle();

	// Calc size
	VkDeviceSize shaderBindingTableSize = groupHandle.handleSizeAligned*2;

	// Create sbt buffer
	Buffer stagingBuffer{};
	createBufferWithAllocator(context->device, context->allocator,
		shaderBindingTableSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer.buffer, stagingBuffer.allocation);

	//createBufferNoAllocator(context->device, context->physDevice,
	//	shaderBindingTableSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	//	sbts.miss.buf.buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sbts.miss.buf.memory);

	rm->addToDeleteQueue(&stagingBuffer, 1);

	// Fill sbt buffer
	void* pMissGroupShaderHandle = groupHandle.handles.data() + groupHandle.handleSize * shaderGroupIndices.miss;
	void* pShadowMissGroupShaderHandle = groupHandle.handles.data() + groupHandle.handleSize * shaderGroupIndices.miss_shadow;
	void* data;
	context->allocator->mapMemory(stagingBuffer.allocation, &data);
	//vkMapMemory(context->device, sbts.miss.buf.memory, 0, shaderBindingTableSize, 0, &data);
	memcpy(data, pMissGroupShaderHandle, groupHandle.handleSize);
	memcpy((char*)data + groupHandle.handleSizeAligned, pShadowMissGroupShaderHandle, groupHandle.handleSize);
	//vkUnmapMemory(context->device, sbts.miss.buf.memory);
	context->allocator->unmapMemory(stagingBuffer.allocation);

	// Copy sbt buffer
	createBufferWithAllocator(context->device, context->allocator,
		shaderBindingTableSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY, sbts.miss.buf.buffer, sbts.miss.buf.allocation);
	rm->addToDeleteQueue(&sbts.miss.buf, 1);

	recordCopyBuffer(cmdBuf, stagingBuffer.buffer, sbts.miss.buf.buffer, shaderBindingTableSize);

	// Set region
	sbts.miss.region.deviceAddress = getBufferAddressKHR(context, sbts.miss.buf.buffer);
	sbts.miss.region.stride = groupHandle.handleSizeAligned;
	sbts.miss.region.size = shaderBindingTableSize;
}

// Not used yet
void PathTraceRenderStage::buildCallableSBT()
{
	sbts.callable = {};
}

void PathTraceRenderStage::buildShaderBindingTables(VkCommandBuffer cmdBuf, std::vector<DrawInst*> instances)
{
	buildHitSBT(cmdBuf, instances);
	buildGenerationSBT(cmdBuf);
	buildMissSBT(cmdBuf);
	buildCallableSBT();
}

void PathTraceRenderStage::createOffscreenImage()
{
	VkImageCreateInfo imgCI{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imgCI.imageType = VK_IMAGE_TYPE_2D;
	imgCI.format = VK_FORMAT_R32G32B32A32_SFLOAT;//context->swapChainImageFormat;
	imgCI.extent.width = context->width;
	imgCI.extent.height = context->height;
	imgCI.extent.depth = 1;
	imgCI.mipLevels = 1;
	imgCI.arrayLayers = 1;
	imgCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imgCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imgCI.usage = VK_IMAGE_USAGE_STORAGE_BIT;// | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	imgCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imgCI.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	createImage(
		context->device,
		context->allocator,
		imgCI,
		offscreenImage);

	VkImageViewCreateInfo imageViewCI{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCI.image = offscreenImage.image;
	imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCI.format = VK_FORMAT_R32G32B32A32_SFLOAT;//context->swapChainImageFormat;
	imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCI.subresourceRange.baseMipLevel = 0;
	imageViewCI.subresourceRange.levelCount = 1;
	imageViewCI.subresourceRange.baseArrayLayer = 0;
	imageViewCI.subresourceRange.layerCount = 1;
	createImageView(
		context->device,
		imageViewCI,
		offscreenImage.view);

	rm->immediateImageLayoutTransition(offscreenImage, VK_IMAGE_LAYOUT_GENERAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);

	descriptorSetManager->addDescriptorUpdate({ offscreenImage }, descriptorSetManager->resourceBindings.offscreenImage, true);
}


void PathTraceRenderStage::recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params)
{
	VK_LOAD(vkCmdTraceRaysKHR);

	uint32_t idx = context->currentSwapchainIndex;
	Image swapChainImage{};
	swapChainImage.image	= context->swapChainImages[idx].image;
	swapChainImage.view		= context->swapChainImages[idx].image_view;
	swapChainImage.layout	= VK_IMAGE_LAYOUT_UNDEFINED;

	buildShaderBindingTables(cmdBuf, params.inst);
	
	// Barrier ---------------------------------------
	VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0, 1, &barrier, 0, nullptr, 0, nullptr);

	//recordCopyImage(cmdBuf, swapChainImage, offscreenImage, context->width, context->height, 1);

	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);

	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSet->set[idx], 0, 0);

	{
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarrier.pNext = NULL;
		imageMemoryBarrier.oldLayout = offscreenImage.layout;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageMemoryBarrier.image = offscreenImage.image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	}


	pvkCmdTraceRaysKHR(cmdBuf, &sbts.generation.region, &sbts.miss.region, &sbts.hit.region, &sbts.callable.region, context->width, context->height, 1);

	{
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarrier.pNext = NULL;
		imageMemoryBarrier.oldLayout = offscreenImage.layout;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageMemoryBarrier.image = offscreenImage.image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	}

	//recordCopyImage(cmdBuf, offscreenImage, swapChainImage, offscreenImage.layout, VK_IMAGE_LAYOUT_GENERAL, context->width, context->height, 1);
	//recordCopyImage(cmdBuf, offscreenImage, swapChainImage, offscreenImage.layout, VK_IMAGE_LAYOUT_GENERAL, context->width, context->height, 1);
}