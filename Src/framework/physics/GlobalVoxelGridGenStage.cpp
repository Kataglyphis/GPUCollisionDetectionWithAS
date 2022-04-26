#include "GlobalVoxelGridGenStage.h"
#include "PipelineToolKit.h"
#include "DrawGeom.h"

GlobalVoxelGridGenStage::GlobalVoxelGridGenStage(Context* context, DescriptorSet* descriptorSet)
{
	this->descriptorSet = descriptorSet;
	this->context = context;
	createPipeline();
}

GlobalVoxelGridGenStage::~GlobalVoxelGridGenStage()
{
}

void GlobalVoxelGridGenStage::destroyPipeline()
{
	vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
	vkDestroyPipeline(context->device, pipeline, nullptr);
}

void GlobalVoxelGridGenStage::createPipeline()
{
	compileShader(context->device, shader, &shaderModule);
	vk_init::ComputePipelineConfiguration conf{};
	conf.shader = shaderModule;
	conf.workGroupSize = workGroupSizeX;
	conf.pushConstantSize = sizeof(GlobalVoxelGridGenStage::PushConstants);
	conf.pDescriptorSetLayout = &descriptorSet->layout;
	vk_init::createPipeline(context->device, conf, &pipelineLayout, &pipeline);
	vkDestroyShaderModule(context->device, shaderModule, nullptr);
}

void GlobalVoxelGridGenStage::recordRenderCommands(VkCommandBuffer& cmdBuf, ComputeParams params)
{
	uint32_t instanceCount = params.geom->instances.size();
	uint32_t voxelCount = params.geom->numVoxels;
	uint32_t numWorkGroups = (instanceCount * voxelCount + workGroupSizeX - 1) / workGroupSizeX;

	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);

	GlobalVoxelGridGenStage::PushConstants pc{};
	pc.numVertecies = voxelCount;
	//pc.vertexAddress = getBufferAddress(context->device, params.geom->vertexBuffer.buffer);
	pc.vertexAddress = params.geom->voxelGridBufferAddress;
	pc.centerOfMass = glm::vec4(params.geom->model->centerOfMass, 0.0);
	pc.numInstances = instanceCount;
	pc.gridCellSize = globalVoxelGridCellSize;
	pc.gridExtent = globalVoxelGridExtent;
	vkCmdPushConstants(cmdBuf, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(GlobalVoxelGridGenStage::PushConstants), &pc);
	vkCmdDispatch(cmdBuf, numWorkGroups, 1, 1);
}
