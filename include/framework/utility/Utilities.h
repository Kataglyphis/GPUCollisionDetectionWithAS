#pragma once

#include <fstream>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include <regex>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "SetsAndBindings.h"
#include "Common.h"
#include "Resources.h"

#include "vk_mem_alloc.h"
#include "Allocator.h"
#include <stb_image.h>
#include "Context.h"






#define GLSLC_COMMAND "glslc.exe --target-env=vulkan1.2"
// defined in cmake
//#define SHADER_SRC_DIR  "../Resources/Shader"
//#define SHADER_SPV_DIR  "../Resources/Shader/spv"
//#define SHADER_LOG_DIR  "../Resources/Shader/log"

template <typename M, typename V>
static void MapToVec(const  M& m, V& v) {
	v.clear();
	for (typename M::const_iterator it = m.begin(); it != m.end(); ++it) {
		v.push_back(it->second);
	}
}

static std::vector<char> read_file(const std::string& filename) {

	// open stream from given file 
	// std::ios::binary tells stream to read file as binary
	// std::ios:ate tells stream to start reading from end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	// check if file stream sucessfully opened
	if (!file.is_open()) {

		std::cout << "Failed to open a file: " << filename << std::endl;
		throw std::runtime_error("Failed to open a file!");

	}

	size_t file_size = (size_t) file.tellg();
	std::vector<char> file_buffer(file_size);

	// move read position to start of file 
	file.seekg(0);

	// read the file data into the buffer (stream "file_size" in total)
	file.read(file_buffer.data(), file_size);

	file.close();

	return file_buffer;
}

static VkShaderModule create_shader_module(VkDevice logical_device, const std::vector<char>& code)
{
	// shader module create info
	VkShaderModuleCreateInfo shader_module_create_info{};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.codeSize = code.size();																											// size of code
	shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());											// pointer to code 

	VkShaderModule shader_module;
	VkResult result = vkCreateShaderModule(logical_device, &shader_module_create_info, nullptr, &shader_module);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a shader module!");

	}

	return shader_module;

}

static uint32_t find_memory_type_index(VkPhysicalDevice physical_device, uint32_t allowed_types, VkMemoryPropertyFlags properties)
{

	// get properties of physical device memory
	VkPhysicalDeviceMemoryProperties memory_properties{};
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {

		if ((allowed_types & (1 << i))																																// index of memory type must match corresponding bit in allowedTypes
			&& (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {					// desired property bit flags are part of memory type's property flags																			

			// this memory type is valid, so return its index
			return i;

		}

	}

	return -1;

}

static void create_buffer(VkPhysicalDevice physical_device, VkDevice device, VkDeviceSize buffer_size, VkBufferUsageFlags buffer_usage_flags, 
											VkMemoryPropertyFlags buffer_propertiy_flags, VkBuffer* buffer, VkDeviceMemory* buffer_memory) {

	// create vertex buffer
	// information to create a buffer (doesn't include assigning memory)
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = buffer_size;																			// size of buffer (size of 1 vertex * #vertices)
	buffer_info.usage = buffer_usage_flags;															// multiple types of buffer possible, e.g. vertex buffer		
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;						// similar to swap chain images, can share vertex buffers

	VkResult result = vkCreateBuffer(device, &buffer_info, nullptr, buffer);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a buffer!");

	}

	// get buffer memory requirements
	VkMemoryRequirements memory_requirements{};
	vkGetBufferMemoryRequirements(device, *buffer, &memory_requirements);

	// allocate memory to buffer
	VkMemoryAllocateInfo memory_alloc_info{};
	memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_alloc_info.allocationSize = memory_requirements.size;

	uint32_t memory_type_index = find_memory_type_index(physical_device, memory_requirements.memoryTypeBits,
																											buffer_propertiy_flags);
																											//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |		/* memory is visible to CPU side */
																											//VK_MEMORY_PROPERTY_HOST_COHERENT_BIT	/* data is placed straight into buffer */);
	if (memory_type_index < 0) {

		throw std::runtime_error("Failed to find auitable memory type!");

	}

	memory_alloc_info.memoryTypeIndex = memory_type_index;

	// allocate memory to VkDeviceMemory
	result = vkAllocateMemory(device, &memory_alloc_info, nullptr, buffer_memory);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate memory for buffer!");

	}

	// allocate memory to given buffer
	vkBindBufferMemory(device, *buffer, *buffer_memory, 0);

}

static VkCommandBuffer begin_command_buffer(VkDevice device, VkCommandPool command_pool) {

	// command buffer to hold transfer commands
	VkCommandBuffer command_buffer;

	// command buffer details
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = command_pool;
	alloc_info.commandBufferCount = 1;

	// allocate command buffer from pool
	vkAllocateCommandBuffers(device, &alloc_info, & command_buffer);

	// infromation to begin the command buffer record
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;											// we are only using the command buffer once, so set up for one time submit
	
	// begin recording transfer commands
	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;

}

static void end_and_submit_command_buffer(VkDevice device, VkCommandPool command_pool, VkQueue queue, VkCommandBuffer& command_buffer) {

	// end commands
	VkResult result = vkEndCommandBuffer(command_buffer);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to end command buffer!");

	}

	// queue submission information 
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	// submit transfer command to transfer queue and wait until it finishes
	result = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to submit to queue!");

	}

	result = vkQueueWaitIdle(queue);

	if (result != VK_SUCCESS) {

		printf("%i", result);
		throw std::runtime_error("Failed to wait Idle!");

	}

	// free temporary command buffer back to pool
	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);

}

static void copy_buffer(VkDevice device, VkQueue transfer_queue, VkCommandPool transfer_command_pool,
											VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize buffer_size) {

	// create buffer
	VkCommandBuffer command_buffer = begin_command_buffer(device, transfer_command_pool);

	// region of data to copy from and to
	VkBufferCopy buffer_copy_region{};
	buffer_copy_region.srcOffset = 0;
	buffer_copy_region.dstOffset = 0;
	buffer_copy_region.size = buffer_size;
	
	// command to copy src buffer to dst buffer
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &buffer_copy_region);

	end_and_submit_command_buffer(device, transfer_command_pool, transfer_queue, command_buffer);

}

static void copy_image_buffer(VkDevice device, VkQueue transfer_queue, VkCommandPool transfer_command_pool, 
													VkBuffer src_buffer, VkImage image, uint32_t width, uint32_t height) {

	// create buffer
	VkCommandBuffer transfer_command_buffer = begin_command_buffer(device, transfer_command_pool);

	VkBufferImageCopy image_region{};
	image_region.bufferOffset = 0;																						// offset into data
	image_region.bufferRowLength = 0;																				// row length of data to calculate data spacing
	image_region.bufferImageHeight = 0;																			// image height to calculate data spacing
	image_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		// which aspect of image to copy
	image_region.imageSubresource.mipLevel = 0;
	image_region.imageSubresource.baseArrayLayer = 0;
	image_region.imageSubresource.layerCount = 1;
	image_region.imageOffset = {0, 0, 0};																			// offset into image 
	image_region.imageExtent = {width, height, 1};

	// copy buffer to given image
	vkCmdCopyBufferToImage(transfer_command_buffer, src_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_region);

	end_and_submit_command_buffer(device, transfer_command_pool, transfer_queue, transfer_command_buffer);

}

static VkAccessFlags access_flags_for_image_layout(VkImageLayout layout) {

	switch (layout)
	{
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VK_ACCESS_HOST_WRITE_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_ACCESS_TRANSFER_READ_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_ACCESS_SHADER_READ_BIT;
	default:
		return VkAccessFlags();
	}

}

static VkPipelineStageFlags pipeline_stage_for_layout(VkImageLayout oldImageLayout) {

	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;  // We do this to allow queue other than graphic
													// return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;  // We do this to allow queue other than graphic
													// return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VK_PIPELINE_STAGE_HOST_BIT;
	case VK_IMAGE_LAYOUT_UNDEFINED:
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	default:
		return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}

}

static void transition_image_layout(VkDevice device, VkQueue queue, VkCommandPool command_pool, VkImage image, VkImageLayout old_layout,
															VkImageLayout new_layout, uint32_t mip_levels) {

	VkCommandBuffer command_buffer = begin_command_buffer(device, command_pool);

	VkImageMemoryBarrier memory_barrier{};
	memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	memory_barrier.oldLayout = old_layout;
	memory_barrier.newLayout = new_layout;
	memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;							// Queue family to transition from 
	memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;							// Queue family to transition to
	memory_barrier.image = image;																							// image being accessed and modified as part of barrier
	memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;			// aspect of image being altered
	memory_barrier.subresourceRange.baseMipLevel = 0;															// first mip level to start alterations on
	memory_barrier.subresourceRange.levelCount = mip_levels;																// number of mip levels to alter starting from baseMipLevel
	memory_barrier.subresourceRange.baseArrayLayer = 0;														// first layer to start alterations on
	memory_barrier.subresourceRange.layerCount = 1;																// number of layers to alter starting from baseArrayLayer

	// if transitioning from new image to image ready to receive data
	memory_barrier.srcAccessMask = access_flags_for_image_layout(old_layout);
	memory_barrier.dstAccessMask = access_flags_for_image_layout(new_layout);

	VkPipelineStageFlags src_stage = pipeline_stage_for_layout(old_layout);
	VkPipelineStageFlags dst_stage = pipeline_stage_for_layout(new_layout);

	vkCmdPipelineBarrier(

		command_buffer,
		src_stage, dst_stage,				// pipeline stages (match to src and dst accessmask)
		0,													// no dependency flags
		0, nullptr,									// memory barrier count + data
		0, nullptr,									// buffer memory barrier count + data
		1, &memory_barrier				// image memory barrier count + data

	);

	end_and_submit_command_buffer(device, command_pool, queue, command_buffer);

}

static void transition_image_layout_for_command_buffer(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout,
											VkImageLayout new_layout, uint32_t mip_levels, VkImageAspectFlags aspectMask) {

	VkImageMemoryBarrier memory_barrier{};
	memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	memory_barrier.oldLayout = old_layout;
	memory_barrier.newLayout = new_layout;
	memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;		// Queue family to transition from 
	memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;		// Queue family to transition to
	memory_barrier.image = image;										// image being accessed and modified as part of barrier
	memory_barrier.subresourceRange.aspectMask = aspectMask;			// aspect of image being altered
	memory_barrier.subresourceRange.baseMipLevel = 0;					// first mip level to start alterations on
	memory_barrier.subresourceRange.levelCount = mip_levels;			// number of mip levels to alter starting from baseMipLevel
	memory_barrier.subresourceRange.baseArrayLayer = 0;					// first layer to start alterations on
	memory_barrier.subresourceRange.layerCount = 1;						// number of layers to alter starting from baseArrayLayer

	memory_barrier.srcAccessMask = access_flags_for_image_layout(old_layout);
	memory_barrier.dstAccessMask = access_flags_for_image_layout(new_layout);

	VkPipelineStageFlags src_stage = pipeline_stage_for_layout(old_layout);
	VkPipelineStageFlags dst_stage = pipeline_stage_for_layout(new_layout);

	// if transitioning from new image to image ready to receive data


	vkCmdPipelineBarrier(

		command_buffer,
		src_stage, dst_stage,				// pipeline stages (match to src and dst accessmask)
		0,													// no dependency flags
		0, nullptr,									// memory barrier count + data
		0, nullptr,									// buffer memory barrier count + data
		1, &memory_barrier				// image memory barrier count + data

	);

}



static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
													VkDebugUtilsMessageTypeFlagsEXT messageType, 
													const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
													void* pUserData) {

	std::string prefix("");

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		prefix = "VERBOSE: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		prefix = "INFO: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		prefix = "WARNING: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		prefix = "ERROR: ";
	}


	// Display message to default output (console/logcat)
	std::stringstream debugMessage;
	debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "][" << pCallbackData->pMessageIdName << "] : " << pCallbackData->pMessage;
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		std::cerr << debugMessage.str() << "\n";
	}
	else {
		std::cout << debugMessage.str() << "\n";
	}
	fflush(stdout);
	return VK_FALSE;

}



// aligned piece of memory appropiately and when necessary return bigger piece
static uint32_t align_up(uint32_t memory, size_t a) {
	return uint32_t((memory + (uint32_t(a) - 1)) & ~uint32_t(a - 1));
}

static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
}


/////////////////////////////////////////////////////////////////////////// Vma Buffers

static void createBufferWithAllocator(
	VkDevice            device,
	Allocator*			allocator,
	VkDeviceSize        deviceSize,
	VkBufferUsageFlags  vkBufferUsageFlags,
	VmaMemoryUsage      vmaMemoryUsageFlags,
	VkBuffer& buffer,
	VmaAllocation& allocation)
{
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = deviceSize;
	bufferInfo.usage = vkBufferUsageFlags;

	VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
	vmaAllocationCreateInfo.usage = vmaMemoryUsageFlags;
	VmaAllocationInfo* pAllocationInfo = {};
	allocator->createBuffer(&bufferInfo, &vmaAllocationCreateInfo, &buffer, &allocation, pAllocationInfo);
	assert((allocation != nullptr));
}

static void createUnallocatedBuffer(
	VkDevice device,
	VkDeviceSize deviceSize,
	VkBufferUsageFlags bufferUsageFlags,
	VkBuffer& buffer)
{
	VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferCreateInfo.size = deviceSize;
	bufferCreateInfo.usage = bufferUsageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;

	VkResult result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);
	ASSERT_VULKAN(result);
}

static void allocateBuffer(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkBufferUsageFlags bufferUsageFlags,
	VkBuffer& buffer,
	VkMemoryPropertyFlags memoryPropertyFlags,
	VkDeviceMemory& deviceMemory,
	VkMemoryRequirements memoryRequirements)
{
	VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO };
	memoryAllocateFlagsInfo.pNext = nullptr;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	memoryAllocateFlagsInfo.deviceMask = 0;


	if (bufferUsageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	}
	else
	{
		memoryAllocateInfo.pNext = nullptr;
	}
	VkResult result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory);
	ASSERT_VULKAN(result);
}

static void createBufferNoAllocator(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkDeviceSize deviceSize,
	VkBufferUsageFlags bufferUsageFlags,
	VkBuffer& buffer,
	VkMemoryPropertyFlags memoryPropertyFlags,
	VkDeviceMemory& deviceMemory)
{
	// Create Buffer
	createUnallocatedBuffer(device, deviceSize, bufferUsageFlags, buffer);
	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
	// Allocate Buffer
	allocateBuffer(device, physicalDevice, bufferUsageFlags, buffer, memoryPropertyFlags, deviceMemory, memoryRequirements);
	// Bind Buffer
	vkBindBufferMemory(device, buffer, deviceMemory, 0);
}


static void destroyBuffer(VkDevice device, Allocator* allocator, Buffer& buffer)
{
	if (buffer.allocation != nullptr)
	{
		if (allocator == nullptr)
		{
			std::cout << "[utility]: Failed to destroy buffer created with allocator. Allocator not initialised.\n";
			return;
		}
		allocator->destroyBuffer(buffer.buffer, buffer.allocation);
	}
	else
	{
		//std::cout << "[utility]: Please use vma allocator.\n";
		vkDestroyBuffer(device, buffer.buffer, NULL);
		vkFreeMemory(device, buffer.memory, NULL);
	}
}


static VkDeviceAddress getBufferAddress(VkDevice device, VkBuffer buffer)
{
	VkBufferDeviceAddressInfo bufferDeviceAddressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferDeviceAddressInfo.buffer = buffer;
	return vkGetBufferDeviceAddress(device, &bufferDeviceAddressInfo);
}

static VkDeviceAddress getBufferAddressKHR(Context* context, VkBuffer buffer)
{
	VK_LOAD(vkGetBufferDeviceAddressKHR);
	VkBufferDeviceAddressInfo sbtBufferDeviceAddressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	sbtBufferDeviceAddressInfo.buffer = buffer;
	return pvkGetBufferDeviceAddressKHR(context->device, &sbtBufferDeviceAddressInfo);
}

/////////////////////////////////////////////////////////////////////////// Vk Buffers for dedicated allocations


static void createBufferDedicated(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkDeviceSize deviceSize,
	VkBufferUsageFlags bufferUsageFlags,
	VkMemoryPropertyFlags memoryPropertyFlags,
	Buffer& buffer)
{
	// Create Buffer
	createUnallocatedBuffer(device, deviceSize, bufferUsageFlags, buffer.buffer);
	// Allocate Buffer
	VkMemoryRequirements2           memReqs{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
	VkMemoryDedicatedRequirements   dedicatedRegs{ VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS };
	VkBufferMemoryRequirementsInfo2 bufferReqs{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2 };
	bufferReqs.buffer = buffer.buffer;
	memReqs.pNext = &dedicatedRegs;
	vkGetBufferMemoryRequirements2(device, &bufferReqs, &memReqs);
	allocateBuffer(device, physicalDevice, bufferUsageFlags, buffer.buffer, memoryPropertyFlags, buffer.memory, memReqs.memoryRequirements);
	// Bind Buffer
	vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);
}



/////////////////////////////////////////////////////////////////////////// Vma Images

static void createImage(
	VkDevice device,
	Allocator* allocator,
	const VkImageCreateInfo imageCreateInfo,
	Image& image)
{
	VmaAllocationCreateInfo vmaAllocationCreateInfo = {};
	vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocator->createImage(&imageCreateInfo, &vmaAllocationCreateInfo, &image.image, &image.allocation);
	image.layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

static void createImageView(VkDevice device, const VkImageViewCreateInfo imageViewCreateInfo, VkImageView& imageView)
{
	VkResult result = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView);
	ASSERT_VULKAN(result);
}

static void destroyImage(VkDevice device, Allocator* allocator, Image image)
{
	if (image.allocation != nullptr)
	{
		if (allocator == nullptr)
		{
			std::cout << "[utility]: Failed to destroy image created with allocator. Allocator not initialised.\n";
			return;
		}
		allocator->destroyImage(image.image, image.allocation);
	}
	else
	{
		std::cout << "[utility]: Please use vma allocator.\n";
		vkDestroyImage(device, image.image, NULL);
		vkFreeMemory(device, image.memory, NULL);
	}
}

static void recordCopyImage(VkCommandBuffer& cmdBuf, Image src, Image dst, VkImageLayout srcNewLayout, VkImageLayout dstNewLayout, float width, float height, uint32_t mipLevels)
{
	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = mipLevels;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	{
		VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarrier.pNext = NULL;
		imageMemoryBarrier.oldLayout = src.layout;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.image = src.image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	}

	{
		VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarrier.pNext = NULL;
		imageMemoryBarrier.oldLayout = dst.layout;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.image = dst.image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	}

	{
		VkImageSubresourceLayers subresourceLayers = {};
		subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceLayers.mipLevel = 0;
		subresourceLayers.baseArrayLayer = 0;
		subresourceLayers.layerCount = 1;

		VkOffset3D offset{};
		offset.x = 0;
		offset.y = 0;
		offset.z = 0;

		VkExtent3D extent{};
		extent.width = width;
		extent.height = height;
		extent.depth = 1;

		VkImageCopy imageCopy{};
		imageCopy.srcSubresource = subresourceLayers;
		imageCopy.srcOffset = offset;
		imageCopy.dstSubresource = subresourceLayers;
		imageCopy.dstOffset = offset;
		imageCopy.extent = extent;

		vkCmdCopyImage(cmdBuf, src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
	}

	{
		VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarrier.pNext = NULL;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.newLayout = srcNewLayout;
		imageMemoryBarrier.image = src.image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	}

	{
		VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarrier.pNext = NULL;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = dstNewLayout;
		imageMemoryBarrier.image = dst.image;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	}
}

/////////////////////////////////////////////////////////////////////////// Record Commands

static void recordCopyBuffer(VkCommandBuffer cmdBuff, VkBuffer src, VkBuffer dest, VkDeviceSize size) {
	VkBufferCopy bufferCopy;
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = size;
	vkCmdCopyBuffer(cmdBuff, src, dest, 1, &bufferCopy);
}

static void recordCopyBufferToImage(VkCommandBuffer cmdBuff, VkBuffer src, VkImage dest, VkBufferImageCopy* pBufferImageCopy) {
	vkCmdCopyBufferToImage(cmdBuff, src, dest, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, pBufferImageCopy);
}



/////////////////////////////////////////////////////////////////////////// Submit Command Buffers

static void submit(const VkCommandBuffer* cmds, uint32_t count, VkQueue queue)
{
	VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submit.pCommandBuffers = cmds;
	submit.commandBufferCount = (uint32_t)count;
	VkResult result = vkQueueSubmit(queue, 1, &submit, nullptr);
	ASSERT_VULKAN(result);
}


// ToDo fix
static void submit(
	const VkCommandBuffer* cmds, uint32_t count, VkQueue queue, SubmitSynchronizationInfo syncInfo)
{
	VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submit.waitSemaphoreCount = syncInfo.waitSemaphoreCount;
	submit.pWaitSemaphores = syncInfo.pWaitSemaphore;
	submit.pWaitDstStageMask = syncInfo.pWaitDstStageMask;
	submit.signalSemaphoreCount = syncInfo.signalSemaphoreCount;
	submit.pSignalSemaphores = syncInfo.pSignalSemaphore;
	submit.pCommandBuffers = cmds;
	submit.commandBufferCount = (uint32_t)count;
	VkResult result = vkQueueSubmit(queue, 1, &submit, syncInfo.signalFence);
	ASSERT_VULKAN(result);
}

/////////////////////////////////////////////////////////////////////////// Alloc/Beginn Command Buffers

static void allocCmdBuf(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer* pCmdBuf, uint32_t count)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = cmdPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = count;

	VkResult result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, pCmdBuf);
	ASSERT_VULKAN(result);
}

static void beginCmdBuf(VkCommandBuffer* pCmdBuf, uint32_t count, VkCommandBufferUsageFlags flags)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = flags;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;
	for (uint32_t i = 0; i < count; i++)
	{
		VkResult result = vkBeginCommandBuffer(pCmdBuf[i], &commandBufferBeginInfo);
		ASSERT_VULKAN(result);
	}
}

/////////////////////////////////////////////////////////////////////////// Upload/Download device data stuff



static void writeToBuffer(VkDevice device, Allocator* allocator, Buffer& buffer, void* data, uint32_t dataSize)
{
	if (buffer.allocation == nullptr)
	{
		std::cout << "Please use allocator\n";
		void* bufferData;
		vkMapMemory(device, buffer.memory, 0, dataSize, 0, &bufferData);
		memcpy(bufferData, data, dataSize);
		vkUnmapMemory(device, buffer.memory);
	}
	else
	{
		void* bufferData;
		allocator->mapMemory(buffer.allocation, &bufferData);
		memcpy(bufferData, data, dataSize);
		allocator->unmapMemory(buffer.allocation);
	}
}

static void readFromBuffer(VkDevice device, Allocator* allocator, Buffer& buffer, void* data, uint32_t dataSize)
{
	if (buffer.allocation == nullptr)
	{
		void* bufferData;
		vkMapMemory(device, buffer.memory, 0, dataSize, 0, &bufferData);
		memcpy(data, bufferData, dataSize);
		vkUnmapMemory(device, buffer.memory);
	}
	else
	{
		void* bufferData;
		allocator->mapMemory(buffer.allocation, &bufferData);
		memcpy(data, bufferData, dataSize);
		allocator->unmapMemory(buffer.allocation);
	}
}

template <typename T>
static void writeVectorToBuffer(VkDevice device, Allocator* allocator, Buffer& buffer, std::vector<T> input)
{
	writeToBuffer(device, allocator, buffer, input.data(), input.size() * sizeof(T));
}

template <typename T>
static void readVectorFromBuffer(VkDevice device, Allocator* allocator, Buffer buffer, std::vector<T>& input)
{
	readFromBuffer(device, allocator, buffer, input.data(), input.size() * sizeof(T));
}

template <typename T>
static void recordUploadToDeviceBuffer(VkDevice device, Allocator* allocator, VkCommandBuffer cmdBuff, Buffer& buffer, const std::vector<T>& input, Buffer& stagingBuffer)
{
	// Create staging buffer
	VkDeviceSize dataSize = input.size() * sizeof(T);
	createBufferWithAllocator(device, allocator, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer.buffer, stagingBuffer.allocation);

	// Write data
	writeVectorToBuffer(device, allocator, stagingBuffer, input);

	// record copy command
	recordCopyBuffer(cmdBuff, stagingBuffer.buffer, buffer.buffer, dataSize);
}

static void recordUploadDataToDeviceBuffer(VkDevice device, Allocator* allocator, VkCommandBuffer cmdBuff, Buffer& buffer, void* data, uint32_t size, Buffer& stagingBuffer)
{
	createBufferWithAllocator(device, allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer.buffer, stagingBuffer.allocation);

	// Write data
	writeToBuffer(device, allocator, stagingBuffer, data, size);

	// record copy command
	recordCopyBuffer(cmdBuff, stagingBuffer.buffer, buffer.buffer, size);
}

template <typename T>
static void recordDownloadToHostBuffer(VkDevice device, Allocator* allocator, VkCommandBuffer cmdBuff, Buffer& buffer, std::vector<T>& input, Buffer& hostBuffer)
{
	// Create staging buffer
	VkDeviceSize dataSize = input.size() * sizeof(T);
	createBufferWithAllocator(device, allocator, dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY, hostBuffer.buffer, hostBuffer.allocation);

	// record copy command
	recordCopyBuffer(cmdBuff, buffer.buffer, hostBuffer.buffer, dataSize);
}

// we have to create mipmap levels in staging buffers by our own
static void recordGenerateMipmaps(VkPhysicalDevice physical_device, VkDevice device, VkCommandBuffer cmdBuff, VkImage image, VkFormat image_format,
	int32_t width, int32_t height, uint32_t mip_levels) {

	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physical_device, image_format, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}


	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	// TEMP VARS needed for decreasing step by step for factor 2
	int32_t tmp_width = width;
	int32_t tmp_height = height;

	// -- WE START AT 1 ! 
	for (uint32_t i = 1; i < mip_levels; i++) {

		// WAIT for previous mip map level for being ready
		barrier.subresourceRange.baseMipLevel = i - 1;
		// HERE we TRANSITION for having a SRC format now 
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuff,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		// when barrier over we can now blit :)
		VkImageBlit blit{};

		// -- OFFSETS describing the 3D-dimesnion of the region 
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { tmp_width, tmp_height, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// copy from previous level
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		// -- OFFSETS describing the 3D-dimesnion of the region 
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { tmp_width > 1 ? tmp_width / 2 : 1, tmp_height > 1 ? tmp_height / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// -- COPY to next mipmap level
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(cmdBuff,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		// REARRANGE image formats for having the correct image formats again
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuff,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (tmp_width > 1) tmp_width /= 2;
		if (tmp_height > 1) tmp_height /= 2;

	}

	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(cmdBuff,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

}

static void recordUploadStbImage(VkDevice device, VkPhysicalDevice physicalDevice, Allocator* allocator, VkCommandBuffer cmdBuff, StbImageInfo imageInfo, Image& image, Buffer& stagingBuffer)
{
	createBufferWithAllocator(device, allocator, imageInfo.size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer.buffer, stagingBuffer.allocation);

	writeToBuffer(device, allocator, stagingBuffer, imageInfo.pPixels, imageInfo.size);

	createImage(device, allocator, imageInfo.imgCreateInfo, image);

	transition_image_layout_for_command_buffer(cmdBuff, image.image, image.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageInfo.imgCreateInfo.mipLevels, imageInfo.aspectMask);

	VkBufferImageCopy bufferImageCopy = {};
	bufferImageCopy.imageSubresource.aspectMask = imageInfo.aspectMask;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageExtent = imageInfo.imgCreateInfo.extent;

	recordCopyBufferToImage(cmdBuff, stagingBuffer.buffer, image.image, &bufferImageCopy);

	recordGenerateMipmaps(physicalDevice, device, cmdBuff, image.image, imageInfo.imgCreateInfo.format, imageInfo.imgCreateInfo.extent.width, imageInfo.imgCreateInfo.extent.height, imageInfo.imgCreateInfo.mipLevels);

	VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewCreateInfo.image = image.image;
	imageViewCreateInfo.viewType = imageInfo.viewType;
	imageViewCreateInfo.format = imageInfo.imgCreateInfo.format;
	imageViewCreateInfo.subresourceRange.aspectMask = imageInfo.aspectMask;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.subresourceRange.levelCount = imageInfo.imgCreateInfo.mipLevels;

	createImageView(device, imageViewCreateInfo, image.view);

	image.layout = imageInfo.layout;
}



template <typename T>
static void recordCreateAndUploadDeviceBuffer(VkDevice device, Allocator* allocator, VkCommandBuffer cmdBuff, Buffer& buffer, VkBufferUsageFlags usage, const std::vector<T>& input, Buffer& stagingBuffer)
{
	// Create Buffer
	VkDeviceSize dataSize = input.size() * sizeof(T);
	createBufferWithAllocator(device, allocator, dataSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, buffer.buffer, buffer.allocation);
	buffer.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	// Upload
	recordUploadToDeviceBuffer(device, allocator, cmdBuff, buffer, input, stagingBuffer);

}

static void recordCreateAndUploadDataToDeviceBuffer(VkDevice device, Allocator* allocator, VkCommandBuffer cmdBuff, Buffer& buffer,
	VkBufferUsageFlags usage, void* data, uint32_t size, Buffer& stagingBuffer)
{
	createBufferWithAllocator(device, allocator, size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, buffer.buffer, buffer.allocation);
	buffer.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	// Upload
	recordUploadDataToDeviceBuffer(device, allocator, cmdBuff, buffer, data, size, stagingBuffer);
}

static void createShaderModule(VkDevice device, const std::vector<char>& code, VkShaderModule* shaderModule) {
	VkShaderModuleCreateInfo shaderCreateInfo;
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.pNext = nullptr;
	shaderCreateInfo.flags = 0;
	shaderCreateInfo.codeSize = code.size();
	shaderCreateInfo.pCode = (uint32_t*)code.data();

	VkResult result = vkCreateShaderModule(device, &shaderCreateInfo, nullptr, shaderModule);
	ASSERT_VULKAN(result);
}


static void compileShader(std::string shader)
{
	std::stringstream shader_src_path;
	std::stringstream shader_spv_path;
	std::stringstream shader_log_path;
	std::stringstream cmdShaderCompile;
	shader_src_path << SHADER_SRC_DIR << "/" << shader;
	shader_spv_path << SHADER_SPV_DIR << "/" << shader << ".spv";
	shader_log_path << SHADER_LOG_DIR << "/" << shader << ".log.txt";


	cmdShaderCompile << GLSLC_COMMAND
		<< " -o " << shader_spv_path.str() << " " << shader_src_path.str()
		<< " 2> " << shader_log_path.str();


	system(cmdShaderCompile.str().c_str());
}


// Special method for Jones because he uses spaces in folder names
static void compile_shaders() {

#if defined (_WIN32)

	compileShader("particle_integration.comp");
	compileShader("particle_simulation.comp");
	system("..\\Resources\\Shader\\compile_compute_shader.bat");


#elif defined (__linux__)
	int result_system = system("chmod +x ../Resources/Shader/compile.sh");



#endif

}

static void compileShader(VkDevice device, std::string shader, VkShaderModule* shaderModule)
{
	compileShader(shader);
	std::stringstream shader_spv_path;
	shader_spv_path << SHADER_SPV_DIR << "/" << shader << ".spv";
	std::string shader_spv_path_str = shader_spv_path.str();
	shader_spv_path_str = std::regex_replace(shader_spv_path_str, std::regex("/"), FILE_SEPARATOR);
	auto shaderCode = read_file(shader_spv_path_str);
	createShaderModule(device, shaderCode, shaderModule);
}

//////////////////////////////////////////////////////////////////////////////////////////////// App helper methods


static QueueFamilyIndices get_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices{};

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_family_list(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_list.data());

	// Go through each queue family and check if it has at least 1 of required types
	// we need to keep track th eindex by our own
	int index = 0;
	for (const auto& queue_family : queue_family_list) {

		// first check if queue family has at least 1 queue in that family 
		// Queue can be multiple types defined through bitfield. Need to bitwise AND with VK_QUE_*_BIT to check if has required  type 
		if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {

			indices.graphics_family = index; // if queue family valid, than get index

		}

		if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {

			indices.compute_family = index;

		}

		// check if queue family suppports presentation
		VkBool32 presentation_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentation_support);
		// check if queue is presentation type (can be both graphics and presentation)
		if (queue_family.queueCount > 0 && presentation_support) {

			indices.presentation_family = index;

		}


		// check if queue family indices are in a valid state
		if (indices.is_valid()) {
			break;
		}

		index++;

	}

	return indices;
}
static SwapChainDetails get_swapchain_details(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainDetails swapchain_details{};
	//get the surface capabilities for the given surface on the given physical device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchain_details.surface_capabilities);

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

	// if formats returned, get list of formats
	if (format_count != 0) {

		swapchain_details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, swapchain_details.formats.data());

	}

	uint32_t presentation_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentation_count, nullptr);

	// if presentation modes returned, get list of presentation modes
	if (presentation_count > 0) {

		swapchain_details.presentation_mode.resize(presentation_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentation_count, swapchain_details.presentation_mode.data());

	}

	return swapchain_details;
}


static bool check_instance_extension_support(std::vector<const char*>* check_extensions)
{
	//Need to get number of extensions to create array of correct size to hold extensions
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

	// create a list of VkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

	/*std::cout << "available extensions:\n";

	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << '\n';
	}*/

	// check if given extensions are in list of available extensions 
	for (const auto& check_extension : *check_extensions) {

		bool has_extension = false;

		for (const auto& extension : extensions) {

			if (strcmp(check_extension, extension.extensionName)) {
				has_extension = true;
				break;
			}

		}

		if (!has_extension) {

			return false;

		}

	}

	return true;
}

static bool check_device_extension_support(VkPhysicalDevice device)
{
	uint32_t extension_count = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

	if (extension_count == 0) {
		return false;
	}

	// populate list of extensions 
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

	for (const auto& device_extension : device_extensions) {

		bool has_extension = false;

		for (const auto& extension : extensions) {

			if (strcmp(device_extension, extension.extensionName) == 0) {
				has_extension = true;
				break;
			}

		}

		if (!has_extension) {

			return false;

		}
	}

	return true;
}

static bool check_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	//Information about device itself (ID, name, type, vendor, etc)
	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(device, &device_properties);

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(device, &device_features);

	QueueFamilyIndices indices = get_queue_families(device, surface);

	bool extensions_supported = check_device_extension_support(device);

	bool swap_chain_valid = false;

	if (extensions_supported) {

		SwapChainDetails swap_chain_details = get_swapchain_details(device, surface);
		swap_chain_valid = !swap_chain_details.presentation_mode.empty() && !swap_chain_details.formats.empty();

	}


	return indices.is_valid() && extensions_supported && swap_chain_valid && device_features.samplerAnisotropy;
}




static VkSurfaceFormatKHR choose_best_surface_format(const std::vector<VkSurfaceFormatKHR>& formats)
{
	//best format is subjective, but I go with:
	// Format:           VK_FORMAT_R8G8B8A8_UNORM (backup-format: VK_FORMAT_B8G8R8A8_UNORM)
	// color_space:  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	// the condition in if means all formats are available (no restrictions)
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {

		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	}

	// if restricted, search  for optimal format
	for (const auto& format : formats) {

		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {

			return format;

		}

	}

	//in case just return first one--- but really shouldn't be the case ....
	return formats[0];
}

static VkPresentModeKHR choose_best_presentation_mode(const std::vector<VkPresentModeKHR>& presentation_modes)
{
	// look for mailbox presentation mode 
	for (const auto& presentation_mode : presentation_modes) {

		if (presentation_mode == VK_PRESENT_MODE_MAILBOX_KHR) {

			return presentation_mode;

		}

	}

	// if can't find, use FIFO as Vulkan spec says it must be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& surface_capabilities, GLFWwindow* window)
{
	// if current extent is at numeric limits, than extent can vary. Otherwise it is size of window
	if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {

		return surface_capabilities.currentExtent;

	}
	else {

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// create new extent using window size
		VkExtent2D new_extent{};
		new_extent.width = static_cast<uint32_t>(width);
		new_extent.height = static_cast<uint32_t>(height);

		// surface also defines max and min, so make sure within boundaries bly clamping value
		new_extent.width = std::max(surface_capabilities.minImageExtent.width, std::min(surface_capabilities.maxImageExtent.width, new_extent.width));
		new_extent.height = std::max(surface_capabilities.minImageExtent.height, std::min(surface_capabilities.maxImageExtent.height, new_extent.height));

		return new_extent;

	}
}

static VkFormat choose_supported_format(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags feature_flags, VkPhysicalDevice physical_device)
{
	// loop through options and find compatible one
	for (VkFormat format : formats) {

		// get properties for give format on this device
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

		// depending on tiling choice, need to check for different bit flag
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & feature_flags) == feature_flags) {

			return format;

		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & feature_flags) == feature_flags) {

			return format;

		}

	}

	throw std::runtime_error("Failed to find supported format!");
}

static bool check_validation_layer_support()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

// Descriptors
static void createDescriptorPool(VkDevice device, std::vector<VkDescriptorPoolSize> descriptorPoolSizes, uint32_t setCount, VkDescriptorPool* descriptorPool)
{
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = nullptr;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = setCount;
	descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	VkResult result = vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, descriptorPool);
	ASSERT_VULKAN(result);
}


static void setDebugMarker(VkDevice device , VkObjectType type, uint64_t obj, const char* name)
{
	PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT =
		(PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");


	const VkDebugUtilsObjectNameInfoEXT objNameInfo =
	{
		VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, // sType
		NULL,                                           // pNext
		type,                                           // objectType
		(uint64_t)obj,                                  // object eg: VK_OBJECT_TYPE_IMAGE,
		name,                                           // pObjectName
	};

	pfnSetDebugUtilsObjectNameEXT(device, &objNameInfo);
}

static std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

static float stringToFloat(std::string s)
{
	if (s.find_first_of(",") != std::string::npos)
		s.replace(s.find_first_of(","), 1, ".");
	float out;
	try
	{
		out = std::stof(s);
	}
	catch (const std::invalid_argument)
	{
		out = 1.0f;
	}
	return out;
}

static float stringToInt(std::string s)
{
	int out;
	try
	{
		out = std::stoi(s);
	}
	catch (const std::invalid_argument)
	{
		out = 1;
	}
	return out;
}