#pragma once
#include "Context.h"
#include <string>
#include <vector>
#include <map>
#include "Common.h"
#include "Resources.h"

class ResourcePool
{
public:
	ResourcePool(Context* pContext, uint32_t queueFamilyIndex, uint32_t queueIndex);
	~ResourcePool();

	void reset();

	VkCommandBuffer startSingleTimeCmdBuffer();
	void endSingleTimeCmdBuffer(VkCommandBuffer cmdBuf);

	VkCommandBuffer cmdBufStart(VkCommandBufferUsageFlags usage);
	std::vector<VkCommandBuffer> cmdBufStart(uint32_t count, VkCommandBufferUsageFlags usage);

	void cmdBufEndAndSubmit(VkCommandBuffer* pCmdBuf, uint32_t count, bool waitForExecution);
	void cmdBufEndAndSubmitSynchronized(VkCommandBuffer* pCmdBuf, uint32_t count, SubmitSynchronizationInfo syncInfo);
	void cmdBufSubmitSynchronized(VkCommandBuffer* pCmdBuf, uint32_t count, SubmitSynchronizationInfo syncInfo);
	void present(std::vector<VkSemaphore> signalSemaphores, uint32_t imageIndex);

	void addToDeleteQueue(VkCommandBuffer* pCmdBuf, uint32_t count);
	void addToDeleteQueue(Buffer* pCmdBuf, uint32_t count);
	void addToDeleteQueue(VkAccelerationStructureKHR* pCmdBuf, uint32_t count);

private:
	Context* context;
	VkQueue						queue;
	VkCommandPool				cmdPool;

	std::map<std::string, VkCommandBuffer>				cmdBufs;
	std::map<std::string, std::vector<VkCommandBuffer>>	cmdBufGroups;


	std::vector<Buffer>									garbageBufferList;
	std::vector<VkCommandBuffer>						garbageCmdBufferList;
	std::vector<VkAccelerationStructureKHR>				garbageAsList;

};
