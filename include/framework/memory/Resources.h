#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"


struct Buffer
{
    VkBuffer        buffer;
    VmaAllocation   allocation{ 0 }; // Vma
    VmaMemoryUsage  memoryUsage; // Vma
    VkBufferView    view; // Needed in some cases

    VkDeviceMemory  memory; // For standard vulkan allocations, try to use vma allocator, when possible.
};

struct Image
{
    VkImage         image;
    VmaAllocation   allocation{ 0 }; // Vma
    VkImageView     view;
    VkImageLayout   layout;

    VkDeviceMemory  memory; // For standard vulkan allocations, try to use vma allocator, when possible.
};

//SH: modified
struct BLAS {

    VkAccelerationStructureKHR as;
    Buffer buffer;
    VkDeviceSize buildScratchSize;
};