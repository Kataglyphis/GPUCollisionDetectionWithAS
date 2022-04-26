#pragma once
#include "DescriptorSet.h"
#include "Context.h"
#include "DrawGeom.h"
#include "ComputeParams.h"
#include "ComputeStage.h"

class CollisionComputeStage : public ComputeStage<ComputeParams>
{
public:

	struct PushConstants
	{
		glm::vec4			centerOfMass;
		VkDeviceAddress		vertexAddress;
		uint32_t			numInstances;
		uint32_t			customIntersectionIndexOffset;
		uint32_t			numVertecies;
	};

	CollisionComputeStage(Context* context, DescriptorSet* descriptorSet);
	~CollisionComputeStage();


	void createPipeline();
	void compileShaders();


	void recordRenderCommands(VkCommandBuffer& cmdBuf, ComputeParams params);

	void destroyPipeline();

private:
	Context* context;
	DescriptorSet* descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	std::string shader = "physics_collision.comp";
	VkShaderModule shaderModule;

	const uint32_t workGroupSizeX = 16;
};
