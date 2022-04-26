#pragma once
#include <glm/glm.hpp>

// All structs must exactly match their shader implementation

struct PerFrameConstants
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	glm::mat4 inverseView;
	glm::mat4 inverseProjection;

	glm::vec4 directionalLight;
	glm::vec4 camPos;

	glm::uint width;
	glm::uint height;
	glm::uint cnt;
	glm::uint placeholder2;

	glm::mat4 particleModel;
};

struct PointLight
{
	glm::vec4 pos;
	glm::vec4 color;
};

struct Material
{
	glm::uint tex_idx_diffuse;
	glm::uint tex_idx_normal;
	glm::uint tex_idx_specular;
	glm::uint tex_idx_coverage;

	glm::uint tex_idx_ambient;
	glm::uint indexOffset;
	glm::uint placeholder2;
	glm::uint placeholder3;
};

struct InstanceDescriptor
{
	glm::mat4 modelMat;

	VkDeviceAddress vertices; // 64 bit

	VkDeviceAddress indices; // 64 bit

	VkDeviceAddress materials; // 64 bit

	VkDeviceAddress placeholder; // 64 bit
};

struct VoxelGridEntry
{
	uint32_t particle1;
	uint32_t particle2;
	uint32_t particle3;
	uint32_t counter;
};