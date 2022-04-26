#pragma once
#include "ResourceManager.h"
#include "DescriptorSetManager.h"
#include "ComputeParams.h"
#include "ComputeStage.h"
class GlobalVoxelGridGenStage : public ComputeStage<ComputeParams>
{
public:

	struct PushConstants
	{
		glm::vec4			centerOfMass;
		VkDeviceAddress		vertexAddress;
		uint32_t			numInstances;
		uint32_t			numVertecies;
		uint32_t			gridExtent;
		float				gridCellSize;
	};
	GlobalVoxelGridGenStage(Context* context, DescriptorSet* descriptorSet);
	~GlobalVoxelGridGenStage();

	void destroyPipeline();
	void createPipeline();
	void recordRenderCommands(VkCommandBuffer& cmdBuf, ComputeParams params);

private:
	Context* context;
	DescriptorSet* descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	std::string shader = "global_voxel_grid_gen.comp";
	VkShaderModule shaderModule;

	const uint32_t workGroupSizeX = 16;
};
