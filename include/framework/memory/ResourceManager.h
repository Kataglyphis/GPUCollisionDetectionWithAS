#pragma once
#include "ResourcePool.h"
#include "Common.h"
#include "Utilities.h"

class DrawGeom;

class ResourceManager
{
public:
	ResourceManager(Context* pContext, int queueFamily);
	~ResourceManager();

	void update();

	VkCommandBuffer startSingleTimeCmdBuffer();

	void endSingleTimeCmdBuffer(VkCommandBuffer cmdBuf);

	VkCommandBuffer cmdBufStart(VkCommandBufferUsageFlags usage);

	std::vector<VkCommandBuffer> cmdBufStart(uint32_t count, VkCommandBufferUsageFlags usage);

	void cmdBufEndAndSubmit(VkCommandBuffer* pCmdBuf, uint32_t count, bool waitForExecution, bool deleteAfterExecution);

	void cmdBufEndAndSubmitSynchronized(VkCommandBuffer* pCmdBuf, uint32_t count, SubmitSynchronizationInfo syncInfo, bool deleteAfterExecution);

	void cmdBufSubmitSynchronized(VkCommandBuffer* pCmdBuf, uint32_t count, SubmitSynchronizationInfo syncInfo, bool deleteAfterExecution);

	void immediateImageLayoutTransition(Image& img, VkImageLayout newLayout, uint32_t mipLevels, VkImageAspectFlags aspectFlags);

	void present(std::vector<VkSemaphore> signalSemaphores, uint32_t imageIndex);

	void immediatUploadStbImages(std::vector<StbImageInfo> imageInfo, std::vector<Image>& images);

	void synchronisedUpdateBLAS(std::vector<DrawGeom*> geom, SubmitSynchronizationInfo syncInfo);

	Buffer createTemporaryBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);

	void addToDeleteQueue(VkCommandBuffer* pCmdBuf, uint32_t count);

	void addToDeleteQueue(Buffer* buf, uint32_t count);

	void addToDeleteQueue(VkAccelerationStructureKHR* as, uint32_t count);

	//time queries
	void cmdResetQueryPool(VkCommandBuffer cmdBuffer);
	bool getQueryResults(std::vector<uint64_t>& queryResults);
	void cmdBeginQuery(VkCommandBuffer cmdBuffer, uint32_t query);
	void cmdEndQuery(VkCommandBuffer cmdBuffer, uint32_t query);
	void cmdWriteTimeStamp(VkCommandBuffer cmdBuffer, uint32_t query);

	void immediatWriteToDeviceBuffer(Buffer& buffer, void* data, VkDeviceSize size, VkDeviceSize offset);

	void createAndUploadDataToDeviceBuffer(VkCommandBuffer cmdBuf, Buffer& buffer, VkBufferUsageFlags usage, void* data, uint32_t size);

	void immediatCreateAndUploadDataToDeviceBuffer(Buffer& buffer, VkBufferUsageFlags usage, void* data, uint32_t size);

	template <typename T>
	void immediateCreateAndUploadDeviceBuffer(Buffer& buffer, VkBufferUsageFlags usage, const std::vector<T>& input)
	{
		VkCommandBuffer cmdBuf = startSingleTimeCmdBuffer();
		Buffer stagingBuffer = {};
		// Record Command
		recordCreateAndUploadDeviceBuffer(context->device, context->allocator, cmdBuf, buffer, usage, input, stagingBuffer);

		// Submit command and wait for completion
		endSingleTimeCmdBuffer(cmdBuf);

		// Delete Staging Buffer
		destroyBuffer(context->device, context->allocator, stagingBuffer);
	}

	

	template <typename T>
	void createAndUploadDeviceBuffer(VkCommandBuffer cmdBuf, Buffer& buffer, VkBufferUsageFlags usage, const std::vector<T>& input)
	{
		Buffer stagingBuffer = {};
		// Record Command
		recordCreateAndUploadDeviceBuffer(context->device, context->allocator, cmdBuf, buffer, usage, input, stagingBuffer);

		// Add to delete queue
		rp_frame[context->currentSwapchainIndex].addToDeleteQueue(&stagingBuffer, 1);
	}

	template <typename T>
	void immediateDownloadDeviceBuffer(Buffer& buffer, std::vector<T>& input)
	{
		VkCommandBuffer cmdBuf = startSingleTimeCmdBuffer();
		Buffer stagingBuffer;
		// Record Command
		recordDownloadToHostBuffer(context->device, context->allocator, cmdBuf, buffer, input, stagingBuffer);

		// Submit command and wait for completion
		endSingleTimeCmdBuffer(cmdBuf);

		// read data
		readVectorFromBuffer(context->device, context->allocator, stagingBuffer, input);

		// Delete Staging Buffer
		destroyBuffer(context->device, context->allocator, stagingBuffer);
	}

	Context* context;

private:

	std::vector<uint64_t>		queryResults;
	VkQueryPool					queryPool;

	// Reset each frame
	std::vector<ResourcePool>	rp_frame;

	// Never reset
	ResourcePool*				rp_global;
};

