#pragma once

#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glm/glm.hpp>
#include <vector>
#include <stb_image.h>

#include "CommonUtility.h"

const int MAX_FRAME_DRAWS = 3;
const int MAX_OBJECTS = 20;
const int NUM_RAYTRACING_DESCRIPTOR_SET_LAYOUTS = 2;
const int NUM_VECTOR_FIELDS = 3;
const int MAX_PEELING_LAYERS = 8;


const uint32_t globalVoxelGridExtent = 400;
const float globalVoxelGridCellSize = 0.02;


#define RAY_TYPES 2
#define PATH_TRACING
//#define USE_DEPTH_PEELING

// use the standard validation layers from the SDK for error checking
const std::vector<const char*> validationLayers = {
					"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

const std::vector<const char*> device_extensions = {

	VK_KHR_SWAPCHAIN_EXTENSION_NAME

};

// DEVICE EXTENSIONS FOR RAYTRACING
const std::vector<const char*> device_extensions_for_raytracing = {

	// Ray query
	VK_KHR_RAY_QUERY_EXTENSION_NAME,
	VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME,
	// raytracing related extensions 
	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	// required from VK_KHR_acceleration_structure
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
	// required for pipeline
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	// required by VK_KHR_spirv_1_4
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
	//required for pipeline library
	VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, 



};

// Indices (locations) of Queue families (if they exist at all)
struct QueueFamilyIndices {

	int graphics_family = -1;																// location of graphics family
	int presentation_family = -1;														// location of presentation queue family
	int compute_family = -1;																// location of compute queue family

	//check if queue families are valid 
	bool is_valid() {

		return graphics_family >= 0 && presentation_family >= 0 && compute_family >= 0;

	}

};

struct MainDevice_ {

	VkPhysicalDevice physical_device;
	VkDevice logical_device;

};

struct SwapChainDetails {

	VkSurfaceCapabilitiesKHR surface_capabilities;				// surface properties, e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;					// surface image formats, e.g. RGBA and size of each color
	std::vector<VkPresentModeKHR> presentation_mode;  // how images should be presented to screen

};

struct SwapChainImage {

	VkImage image;
	VkImageView image_view;

};

struct Particle {

	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 velocity;
	glm::vec4 acceleration;

	// For particle pass
	static VkVertexInputBindingDescription getBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

};

enum ParticleTypes {
	Glue,
	Bounce,
	Element_COUNT
};

struct ParticlePositionLimits {

	uint32_t maxPositionX;
	uint32_t maxPositionY;
	uint32_t maxPositionZ;

};

struct SpecializationData {

	// standard values 
	uint32_t specWorkGroupSizeX = 8;
	uint32_t specWorkGroupSizeY = 8;
	uint32_t specWorkGroupSizeZ = 4;

};

struct ComputeLimits {

	uint32_t maxComputeWorkGroupCount[3];
	uint32_t maxComputeWorkGroupInvocations;
	uint32_t maxComputeWorkGroupSize[3];

};

struct VectorFieldDimensions {
	
	uint32_t x;
	uint32_t y;
	uint32_t z;

	bool is_valid() {

		return x >= 1 && y >= 1 && z >= 1;

	}

};

struct SubmitSynchronizationInfo
{
	uint32_t				waitSemaphoreCount;
	VkSemaphore*			pWaitSemaphore;
	VkPipelineStageFlags*	pWaitDstStageMask;
	uint32_t				signalSemaphoreCount;
	VkSemaphore*			pSignalSemaphore;
	VkFence					signalFence;
};

enum render_passes
{
	DEFERRED_GEOMETRY_PASS,
	DEFERRED_SHADING_PASS
};

struct StbImageInfo
{
	stbi_uc*			pPixels;
	VkDeviceSize		size;
	VkImageCreateInfo	imgCreateInfo;
	VkImageAspectFlags	aspectMask;
	VkImageViewType		viewType;
	VkImageLayout		layout;
};

enum TextureFormat
{
	TEXTURE_FORMAT_DIFFUSE_ALBEDO	= VK_FORMAT_R8G8B8A8_SRGB,
	TEXTURE_FORMAT_COVERAGE			= VK_FORMAT_R8G8B8A8_UNORM,
	TEXTURE_FORMAT_NORMAL			= VK_FORMAT_R8G8B8A8_UNORM,
	TEXTURE_FORMAT_SPECULAR			= VK_FORMAT_R8G8B8A8_UNORM,
	TEXTURE_FORMAT_EMISSIVE			= VK_FORMAT_R8G8B8A8_SRGB,
};

struct DepthPeelingConstants {
	glm::mat4 view;
	glm::mat4 projection;
};

const int MAX_FRAGMENTS		= 8;
const int INSERTION_RATE	= 8;

struct Node
{
	glm::vec4	position;

	float		depth;
	uint32_t	next;

	uint32_t	placeholder1;
	uint32_t	placeholder2;

};

struct Counter {

	uint32_t count;
	uint32_t maxNodeCount;
	uint32_t particle_count;
	uint32_t placeholder1;

};