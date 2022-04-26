#include "ResourcePool.h"
#include <map>
#include "Utilities.h"



ResourcePool::ResourcePool(Context* pContext, uint32_t queueFamilyIndex, uint32_t queueIndex)
{
    context = pContext;
    vkGetDeviceQueue(context->device, queueFamilyIndex, queueIndex, &queue);

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.flags = 0;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

    VkResult result = vkCreateCommandPool(context->device, &commandPoolCreateInfo, nullptr, &cmdPool);
    ASSERT_VULKAN(result);

}

ResourcePool::~ResourcePool()
{
}

void ResourcePool::reset()
{
    VK_LOAD(vkDestroyAccelerationStructureKHR);
    for each (Buffer buffer in garbageBufferList)
    {
        destroyBuffer(context->device, context->allocator, buffer);
    }
    garbageBufferList.clear();
    for each (VkCommandBuffer cmdBuf in garbageCmdBufferList)
    {
        vkFreeCommandBuffers(context->device, cmdPool, 1, &cmdBuf);
    }
    garbageCmdBufferList.clear();
    for each (VkAccelerationStructureKHR as in garbageAsList)
    {
        pvkDestroyAccelerationStructureKHR(context->device, as, nullptr);
    }
    garbageAsList.clear();
    VkResult result = vkResetCommandPool(context->device, cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    ASSERT_VULKAN(result);

}


///////////////////////////////////////////////////////////////////////// Commad buffers


// For easy usage:

VkCommandBuffer ResourcePool::startSingleTimeCmdBuffer()
{
    return cmdBufStart(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

// Contains queue wait idle, dont use on a per frame basis
void ResourcePool::endSingleTimeCmdBuffer(VkCommandBuffer cmdBuf)
{
    cmdBufEndAndSubmit(&cmdBuf, 1, true);
}


// Create command buffer

VkCommandBuffer ResourcePool::cmdBufStart(VkCommandBufferUsageFlags usage)
{
    VkCommandBuffer commandBuffer;
    allocCmdBuf(context->device, cmdPool, &commandBuffer, 1);
    beginCmdBuf(&commandBuffer, 1, usage);
    return commandBuffer;
}

std::vector<VkCommandBuffer> ResourcePool::cmdBufStart(uint32_t count, VkCommandBufferUsageFlags usage)
{
    std::vector<VkCommandBuffer> commandBuffers(count);
    allocCmdBuf(context->device, cmdPool, commandBuffers.data(), count);
    beginCmdBuf(commandBuffers.data(), count, usage);
    return commandBuffers;
}


// Submit command Buffer
void ResourcePool::cmdBufEndAndSubmit(VkCommandBuffer* pCmdBuf, uint32_t count, bool waitForExecution)
{
    for (uint32_t i = 0; i < count; i++)
    {
        vkEndCommandBuffer(pCmdBuf[i]);
    }

    submit(pCmdBuf, count, queue);

    if (waitForExecution)
    {
        VkResult result = vkQueueWaitIdle(queue);
        ASSERT_VULKAN(result);
    }
    addToDeleteQueue(pCmdBuf, count);
}

void ResourcePool::cmdBufEndAndSubmitSynchronized(VkCommandBuffer* pCmdBuf, uint32_t count, SubmitSynchronizationInfo syncInfo)
{
    for (uint32_t i = 0; i < count; i++)
    {
        vkEndCommandBuffer(pCmdBuf[i]);
    }
    cmdBufSubmitSynchronized(pCmdBuf, count, syncInfo);
}

void ResourcePool::cmdBufSubmitSynchronized(VkCommandBuffer* pCmdBuf, uint32_t count, SubmitSynchronizationInfo syncInfo)
{
    submit(pCmdBuf, count, queue, syncInfo);
    addToDeleteQueue(pCmdBuf, count);
}

void ResourcePool::addToDeleteQueue(VkCommandBuffer* pCmdBuf, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        garbageCmdBufferList.push_back(pCmdBuf[i]);
    }
}

void ResourcePool::addToDeleteQueue(Buffer* buf, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        garbageBufferList.push_back(buf[i]);
    }
}

void ResourcePool::addToDeleteQueue(VkAccelerationStructureKHR* as, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        garbageAsList.push_back(as[i]);
    }
}

void ResourcePool::present(std::vector<VkSemaphore> signalSemaphores, uint32_t imageIndex)
{
    VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = signalSemaphores.size();
    presentInfo.pWaitSemaphores = signalSemaphores.data();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &context->swapchain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(queue, &presentInfo);
}

