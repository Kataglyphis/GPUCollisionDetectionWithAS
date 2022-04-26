
#include "ResourceManager.h"
#include "DrawGeom.h"

ResourceManager::ResourceManager(Context* pContext, int queueFamily)
{
	context = pContext;
	// One resource pool per swapchain image
	for (uint32_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		rp_frame.push_back(ResourcePool(pContext, queueFamily, 0));
	}
	// One static resource pool (compute/graphics)
	rp_global = new ResourcePool(pContext, queueFamily, 0);

	//create query pool for time stamps 
	queryResults.resize(context->queryCount);

	VkQueryPoolCreateInfo queryPoolInfo = {};
	queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	// This query pool will store pipeline statistics
	queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
	// Pipeline counters to be returned for this pool
	queryPoolInfo.pipelineStatistics =
		VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
	queryPoolInfo.queryCount = context->queryCount;
	VkResult result = vkCreateQueryPool(context->device, &queryPoolInfo, NULL, &queryPool);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create query pool!");
	}
}

ResourceManager::~ResourceManager()
{
}



void ResourceManager::update()
{
	rp_frame[context->currentSwapchainIndex].reset();
}



///////////////////////////////////////////////////////////////////////// Command Buffers

// Allocted only for current frame
VkCommandBuffer ResourceManager::startSingleTimeCmdBuffer()
{
	return rp_frame[context->currentSwapchainIndex].startSingleTimeCmdBuffer();
}
void ResourceManager::endSingleTimeCmdBuffer(VkCommandBuffer cmdBuf)
{
	return rp_frame[context->currentSwapchainIndex].endSingleTimeCmdBuffer(cmdBuf);
}

// Allocted Permanently unless deleteAfterExecution=true when submited
VkCommandBuffer ResourceManager::cmdBufStart(VkCommandBufferUsageFlags usage)
{
	return rp_global->cmdBufStart(usage);
}

std::vector<VkCommandBuffer> ResourceManager::cmdBufStart(uint32_t count, VkCommandBufferUsageFlags usage)
{
	return rp_global->cmdBufStart(count, usage);
}

void ResourceManager::cmdBufEndAndSubmit(VkCommandBuffer* pCmdBuf, uint32_t count, bool waitForExecution, bool deleteAfterExecution)
{
	if (deleteAfterExecution)
	{
		rp_frame[context->currentSwapchainIndex].cmdBufEndAndSubmit(pCmdBuf, count, waitForExecution);
	}
	else
	{
		rp_global->cmdBufEndAndSubmit(pCmdBuf, count, waitForExecution);
	}
}

void ResourceManager::cmdBufEndAndSubmitSynchronized(VkCommandBuffer* pCmdBuf, uint32_t count, SubmitSynchronizationInfo syncInfo, bool deleteAfterExecution)
{
	if (deleteAfterExecution)
	{
		rp_frame[context->currentSwapchainIndex].cmdBufEndAndSubmitSynchronized(pCmdBuf, count, syncInfo);
	}
	else
	{
		rp_global->cmdBufEndAndSubmitSynchronized(pCmdBuf, count, syncInfo);
	}
}

void ResourceManager::cmdBufSubmitSynchronized(VkCommandBuffer* pCmdBuf, uint32_t count, SubmitSynchronizationInfo syncInfo, bool deleteAfterExecution)
{
	if (deleteAfterExecution)
	{
		rp_frame[context->currentSwapchainIndex].cmdBufSubmitSynchronized(pCmdBuf, count, syncInfo);
	}
	else
	{
		rp_global->cmdBufSubmitSynchronized(pCmdBuf, count, syncInfo);
	}
}

void ResourceManager::immediateImageLayoutTransition(Image& img, VkImageLayout newLayout, uint32_t mipLevels, VkImageAspectFlags aspectFlags)
{
	ResourcePool* rp = &rp_frame[context->currentSwapchainIndex];
	VkCommandBuffer cmdBuf = rp->startSingleTimeCmdBuffer();
	transition_image_layout_for_command_buffer(cmdBuf, img.image, img.layout, newLayout, mipLevels, aspectFlags);
	img.layout = newLayout;
	rp->endSingleTimeCmdBuffer(cmdBuf);
}

void ResourceManager::present(std::vector<VkSemaphore> signalSemaphores, uint32_t imageIndex)
{
	rp_global->present(signalSemaphores, imageIndex);
}

void ResourceManager::immediatUploadStbImages(std::vector<StbImageInfo> imageInfo, std::vector<Image>& images)
{
	uint32_t imageCount = imageInfo.size();
	std::vector<Buffer> stagingBuffers;
	stagingBuffers.resize(imageCount);
	images.resize(imageCount);
	ResourcePool* rp = &rp_frame[context->currentSwapchainIndex];
	VkCommandBuffer cmdBuff = rp->startSingleTimeCmdBuffer();
	for (uint32_t i = 0; i < imageCount; i++)
	{
		recordUploadStbImage(context->device, context->physDevice, context->allocator, cmdBuff,
			imageInfo[i], images[i], stagingBuffers[i]);
	}
	rp->endSingleTimeCmdBuffer(cmdBuff);
	for (uint32_t i = 0; i < imageCount; i++)
	{
		destroyBuffer(context->device, context->allocator, stagingBuffers[i]);
	};
}

void ResourceManager::synchronisedUpdateBLAS(std::vector<DrawGeom*> geom, SubmitSynchronizationInfo syncInfo)
{
	VkCommandBuffer cmdBuf = startSingleTimeCmdBuffer();
	VkDeviceSize    maxScratch{ 0 };
	Buffer			scratchBuffer{};

	// Get scratch buffer size
	for (uint32_t i = 0; i < geom.size(); i++)
	{
		maxScratch = std::max(maxScratch, geom[i]->blas.buildScratchSize);
	}

	// create scratch buffer
	createBufferDedicated(context->device, context->physDevice,
		maxScratch,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		scratchBuffer);

	// record update as commands
	for (uint32_t i = 0; i < geom.size(); i++)
	{
		if (geom[i]->hasBlas)
		{
			rp_frame[context->currentSwapchainIndex].addToDeleteQueue(&geom[i]->blas.as, 1);
			rp_frame[context->currentSwapchainIndex].addToDeleteQueue(&geom[i]->blas.buffer, 1);
		}
		geom[i]->recordUpdateBLAS(cmdBuf, scratchBuffer);
	}

	// end and submit
	rp_frame[context->currentSwapchainIndex].cmdBufEndAndSubmitSynchronized(&cmdBuf, 1, syncInfo);
	// add scratch buffer to delete queue
	rp_frame[context->currentSwapchainIndex].addToDeleteQueue(&scratchBuffer, 1);
}

Buffer ResourceManager::createTemporaryBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage)
{
	Buffer buffer{};
	createBufferWithAllocator(context->device, context->allocator, size,
		bufferUsage,
		memoryUsage, buffer.buffer, buffer.allocation);
	// add buffer to delete queue
	rp_frame[context->currentSwapchainIndex].addToDeleteQueue(&buffer, 1);
	return buffer;
}

void ResourceManager::addToDeleteQueue(VkCommandBuffer* pCmdBuf, uint32_t count)
{
	rp_frame[context->currentSwapchainIndex].addToDeleteQueue(pCmdBuf, 1);
}

void ResourceManager::addToDeleteQueue(Buffer* buf, uint32_t count)
{
	rp_frame[context->currentSwapchainIndex].addToDeleteQueue(buf, 1);
}

void ResourceManager::addToDeleteQueue(VkAccelerationStructureKHR* as, uint32_t count)
{
	rp_frame[context->currentSwapchainIndex].addToDeleteQueue(as, 1);
}

void ResourceManager::cmdResetQueryPool(VkCommandBuffer cmdBuffer)
{
	vkCmdResetQueryPool(cmdBuffer, queryPool, 0, context->queryCount);
}

bool ResourceManager::getQueryResults(std::vector<uint64_t>& queryResults)
{
	VkResult result = vkGetQueryPoolResults(
		context->device,
		queryPool,
		0,
		context->queryCount,
		queryResults.size() * sizeof(uint64_t),
		queryResults.data(),
		static_cast<VkDeviceSize>(sizeof(uint64_t)),
		VK_QUERY_RESULT_64_BIT);

	if (result == VK_NOT_READY) return false;

	return true;
}

void ResourceManager::cmdBeginQuery(VkCommandBuffer cmdBuffer, uint32_t query)
{
	vkCmdBeginQuery(cmdBuffer, queryPool, query, 0);
}

void ResourceManager::cmdEndQuery(VkCommandBuffer cmdBuffer, uint32_t query)
{
	vkCmdEndQuery(cmdBuffer, queryPool, query);
}

void ResourceManager::cmdWriteTimeStamp(VkCommandBuffer cmdBuffer, uint32_t query)
{
	vkCmdWriteTimestamp(cmdBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
						queryPool, query);
}


void ResourceManager::immediatWriteToDeviceBuffer(Buffer& buffer, void* data, VkDeviceSize size, VkDeviceSize offset)
{
	VkCommandBuffer cmdBuf = startSingleTimeCmdBuffer();
	Buffer stagingBuffer = {};
	// Record Command
	createBufferWithAllocator(context->device, context->allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer.buffer, stagingBuffer.allocation);
	writeToBuffer(context->device, context->allocator, stagingBuffer, data, size);

	VkBufferCopy bufferCopy;
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = offset;
	bufferCopy.size = size;
	vkCmdCopyBuffer(cmdBuf, stagingBuffer.buffer, buffer.buffer, 1, &bufferCopy);

	// Submit command and wait for completion
	endSingleTimeCmdBuffer(cmdBuf);

	// Delete Staging Buffer
	destroyBuffer(context->device, context->allocator, stagingBuffer);
}


void ResourceManager::createAndUploadDataToDeviceBuffer(VkCommandBuffer cmdBuf, Buffer& buffer, VkBufferUsageFlags usage, void* data, uint32_t size)
{
	Buffer stagingBuffer = {};
	// Record Command
	recordCreateAndUploadDataToDeviceBuffer(context->device, context->allocator, cmdBuf, buffer, usage, data, size, stagingBuffer);

	// Delete Staging Buffer
	rp_frame[context->currentSwapchainIndex].addToDeleteQueue(&stagingBuffer, 1);
}

void ResourceManager::immediatCreateAndUploadDataToDeviceBuffer(Buffer& buffer, VkBufferUsageFlags usage, void* data, uint32_t size)
{
	VkCommandBuffer cmdBuf = startSingleTimeCmdBuffer();
	Buffer stagingBuffer = {};
	// Record Command
	recordCreateAndUploadDataToDeviceBuffer(context->device, context->allocator, cmdBuf, buffer, usage, data, size, stagingBuffer);

	endSingleTimeCmdBuffer(cmdBuf);

	// Delete Staging Buffer
	destroyBuffer(context->device, context->allocator, stagingBuffer);
}