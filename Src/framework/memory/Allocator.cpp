#include "Allocator.h"
//#include "Utilities.h"
#include <assert.h>
#include "CommonUtility.h"



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Allocator::Allocator(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance)
{
	init(device, physicalDevice, instance);
}

Allocator::~Allocator()
{
}

void Allocator::init(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance)
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &vmaAllocator);
}

void Allocator::createImage(
	const VkImageCreateInfo* pImageCreateInfo,
	VmaAllocationCreateInfo* pVmaAllocationCreateInfo,
	VkImage* pImage,
	VmaAllocation* pAllocation)
{
	VkResult result = vmaCreateImage(this->vmaAllocator, pImageCreateInfo, pVmaAllocationCreateInfo, pImage, pAllocation, nullptr);
	ASSERT_VULKAN(result);
}

void Allocator::destroyImage(VkImage& image, VmaAllocation& allocation)
{
	vmaDestroyImage(this->vmaAllocator, image, allocation);
}

void Allocator::mapMemory(VmaAllocation& allocation, void** ppData)
{
	vmaMapMemory(this->vmaAllocator, allocation, ppData);
}
void Allocator::unmapMemory(VmaAllocation& allocation)
{
	vmaUnmapMemory(this->vmaAllocator, allocation);
}

void Allocator::createBuffer(
	VkBufferCreateInfo*	pBufferCreateInfo,	
	VmaAllocationCreateInfo* pVmaAllocationCreateInfo,
	VkBuffer* pBuffer,
	VmaAllocation* pAllocation,
	VmaAllocationInfo* pAllocationInfo)
{
	vmaCreateBuffer(this->vmaAllocator, pBufferCreateInfo, pVmaAllocationCreateInfo, pBuffer, pAllocation, pAllocationInfo);
	assert((pAllocation != nullptr));
}

void Allocator::destroyBuffer(VkBuffer& buffer, VmaAllocation& allocation)
{
	vmaDestroyBuffer(this->vmaAllocator, buffer, allocation);
}
