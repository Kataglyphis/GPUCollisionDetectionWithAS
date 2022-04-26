#pragma once
#include "RayTraceStage.h"
#include "DrawParams.h"
#include "Context.h"
#include "ResourceManager.h"

struct ShaderGroupHandle
{
	std::vector<uint8_t>	handles;
	uint32_t				handleSize;
	uint32_t				handleSizeAligned;
	uint32_t				baseAlignement;
};

struct ShaderBindingTable
{
	Buffer buf;
	VkStridedDeviceAddressRegionKHR region;
};

class DescriptorSetManager;

class PathTraceRenderStage : public RayTraceStage<DrawParams>
{
public:
	PathTraceRenderStage(ResourceManager* rm, DescriptorSetManager* descriptorSet);
	void reloadShaders();
	~PathTraceRenderStage();

	void destroyPipeline();
	void createPipeline();
	void recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params);
	void buildShaderBindingTables(VkCommandBuffer cmdBuf, std::vector<DrawInst*> inst);


private:
	ShaderGroupHandle getShaderGroupHandle();

	void buildHitSBT(VkCommandBuffer cmdBuf, std::vector<DrawInst*> instances);

	void buildGenerationSBT(VkCommandBuffer cmdBuf);

	void buildMissSBT(VkCommandBuffer cmdBuf);

	void buildCallableSBT();

	void createOffscreenImage();

	Context* context;
	DescriptorSet* descriptorSet;
	DescriptorSetManager* descriptorSetManager;
	ResourceManager* rm;
	Image offscreenImage;

	struct Shaders
	{
		std::string		any_hit_opaque = "path_trace.opaque.rahit";
		std::string		any_hit_transparent = "path_trace.transparent.rahit";
		std::string		closest_hit = "path_trace.rchit";
		std::string		generation = "path_trace.rgen";
		std::string		miss = "path_trace.rmiss";
		std::string		shadowMiss = "path_trace.shadow.rmiss";
	};
	Shaders					shaders;

	struct ShaderModules
	{
		VkShaderModule		any_hit_opaque;
		VkShaderModule		any_hit_transparent;
		VkShaderModule		closest_hit;
		VkShaderModule		generation;
		VkShaderModule		miss;
		VkShaderModule		shadowMiss;
	};
	ShaderModules			shaderModules;

	struct ShaderGroupIndices
	{
		uint32_t generation = 0;
		uint32_t miss = 1;
		uint32_t miss_shadow = 2;
		uint32_t hit_opaque = 3;
		uint32_t hit_transparent = 4;
	};
	ShaderGroupIndices shaderGroupIndices;
	const uint32_t groupCount = 5;
	const uint32_t rayTypes = RAY_TYPES;

	

	struct SBTs
	{
		ShaderBindingTable generation;
		ShaderBindingTable miss;
		ShaderBindingTable hit;
		ShaderBindingTable callable;
	};
	SBTs sbts;

	

	VkPipeline				pipeline;
	VkPipelineLayout		pipelineLayout;

	const uint32_t			ray_max_recursion_level = 3;
};

