#pragma once
#include "DescriptorSet.h"
#include "Context.h"
#include "DrawGeom.h"
#include "ComputeStage.h"
#include "ComputeParams.h"

class ModelMatrixComputeStage : public ComputeStage<ComputeParams>
{
public:
	struct PushConstants
	{
		uint32_t	numInstances;
		uint32_t	enableGravity;
		float		gravityStrenght;
		uint32_t	enableExplosion;
		glm::vec4	explosionCenter;
		float		explosionStrenght;
	};

	ModelMatrixComputeStage(Context* context, DescriptorSet* descriptorSet);
	~ModelMatrixComputeStage();

	void createPipeline();
	void compileShaders();
	void recordRenderCommands(VkCommandBuffer& cmdBuf, ComputeParams params);

	void destroyPipeline();

private:
	Context* context;
	DescriptorSet* descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	std::string shader = "physics_update_model_matrix.comp";
	VkShaderModule shaderModule;

	const uint32_t workGroupSizeX = 16;
};
