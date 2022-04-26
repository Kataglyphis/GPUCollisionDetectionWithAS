#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#define DELETE_COPY_CONSTRUCTORS(A) A(const A&) = delete;\
									A(A&&) = delete;\
									A& operator=(const A&) = delete;\
									A& operator=(A&&) = delete;
class Allocator
{
public:
	Allocator(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance);
	~Allocator();

	void init(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance);
	void createImage(const VkImageCreateInfo* pImageCreateInfo, VmaAllocationCreateInfo* pVmaAllocationCreateInfo, VkImage* pImage, VmaAllocation* pAllocation);
	void destroyImage(VkImage& image, VmaAllocation& allocation);
	void mapMemory(VmaAllocation& allocation, void** ppData);
	void unmapMemory(VmaAllocation& allocation);
	void createBuffer(VkBufferCreateInfo* pBufferCreateInfo, VmaAllocationCreateInfo* pVmaAllocationCreateInfo, VkBuffer* pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo);
	void destroyBuffer(VkBuffer& buffer, VmaAllocation& allocation);

	DELETE_COPY_CONSTRUCTORS(Allocator)
private:
	VmaAllocator	vmaAllocator;
	VkDevice		device;
};
