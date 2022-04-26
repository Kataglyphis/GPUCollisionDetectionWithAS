#pragma once
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include "Common.h"
#include "Allocator.h"
#include "Resources.h"
#include "MovementStrategy.h"
#include "Config.h"

// Render context

class AsManager;

struct Context
{
	Context() {};
	VkDevice						device;
	GLFWwindow*						window;
	VkPhysicalDevice				physDevice;
	VkInstance						instance;
	Allocator*						allocator;
	uint32_t						currentSwapchainIndex;
	uint32_t						totalFrameIndex;
	QueueFamilyIndices				queueFamilyIndices;
	uint32_t						width;
	uint32_t						height;
	std::vector<SwapChainImage>		swapChainImages;
	VkFormat						swapChainImageFormat;
	VkSwapchainKHR					swapchain;

	std::vector<VkSemaphore>		imageAvailableSemaphores;
	std::vector<VkSemaphore>		renderFinishedSemaphores;
	std::vector<VkSemaphore>		physicsFinishedSemaphores;
	std::vector<VkSemaphore>		uploadFinishedSemaphores;
	std::vector<VkFence>			inFlightFences;
	std::vector<VkFence>			imagesInFlight;

	std::vector<VkSemaphore>		renderComputeSync;
	std::vector<VkSemaphore>		asFinished;
	std::vector<Buffer>				storage_buffer_read_results;

	bool vectorFieldChanged			= false;
	bool numParticlesChanged		= false;
	bool computeUniformChanged		= false;
	bool computeShaderChanged		= false;

	SpecializationData spec_data;
	ComputeLimits compute_limits;

	float workGroupTestingProgess;
	bool workGroupTesting;

	
#ifdef _DEBUG
	uint32_t numParticlesX = 8;
	uint32_t numParticlesY = 8;
	uint32_t numParticlesZ = 8;
#else
	//uint32_t numParticlesX = 128;
	//uint32_t numParticlesY = 128;
	//uint32_t numParticlesZ = 64;
	uint32_t numParticlesX = 32;
	uint32_t numParticlesY = 32;
	uint32_t numParticlesZ = 32;
#endif

	ParticleTypes particleType;

	glm::mat4 particleModel;

	float particleAreaOfInfluenceX;
	float particleAreaOfInfluenceY;
	float particleAreaOfInfluenceZ;
	glm::vec3 particleTranslation;

	ParticlePositionLimits posLimits;

	VectorFieldDimensions vector_field_dim;
	std::vector<MovementStrategy*> vectorFieldStrategies;
	std::vector<glm::vec3> vectorFieldTranslationDirections;
	std::vector<float> vectorFieldVelocityStrength;
	float particleVelocity = 0.1f;

	uint32_t queryCount				= 6;
	float time_simulation_stage_ms	= -1;
	float time_integration_stage_ms = -1;
	float time_compute_pass_ms		= -1;

	uint32_t bestWorkGroupSizeX;
	uint32_t bestWorkGroupSizeY;
	uint32_t bestWorkGroupSizeZ;

	VkPhysicalDeviceProperties physicalDeviceProps;

	AsManager* asManager;

	DELETE_COPY_CONSTRUCTORS(Context)
};