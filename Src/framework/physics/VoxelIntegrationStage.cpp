#include "VoxelIntegrationStage.h"
#include "PipelineToolKit.h"
#include "DrawGeom.h"

VoxelIntegrationStage::VoxelIntegrationStage(Context* context, DescriptorSet* descriptorSet)
{
	this->descriptorSet = descriptorSet;
	this->context = context;
	createPipeline();
}

VoxelIntegrationStage::~VoxelIntegrationStage()
{
}

void VoxelIntegrationStage::destroyPipeline()
{
	vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
	vkDestroyPipeline(context->device, pipeline, nullptr);
}

void VoxelIntegrationStage::createPipeline()
{
	compileShader(context->device, shader, &shaderModule);
	vk_init::ComputePipelineConfiguration conf{};
	conf.shader = shaderModule;
	conf.workGroupSize = workGroupSizeX;
	conf.pushConstantSize = sizeof(VoxelIntegrationStage::PushConstants);
	conf.pDescriptorSetLayout = &descriptorSet->layout;
	vk_init::createPipeline(context->device, conf, &pipelineLayout, &pipeline);
	vkDestroyShaderModule(context->device, shaderModule, nullptr);
}

void VoxelIntegrationStage::recordRenderCommands(VkCommandBuffer& cmdBuf, ComputeParams params)
{
	uint32_t instanceCount = params.geom->instances.size();
	uint32_t voxelCount = params.geom->numVoxels;
	uint32_t numWorkGroups = (instanceCount * voxelCount + workGroupSizeX - 1) / workGroupSizeX;

	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);

	VoxelIntegrationStage::PushConstants pc{};
	pc.numVertecies = voxelCount;
	//pc.vertexAddress = getBufferAddress(context->device, params.geom->vertexBuffer.buffer);
	pc.vertexAddress = params.geom->voxelGridBufferAddress;
	pc.centerOfMass = glm::vec4(params.geom->model->centerOfMass, 0.0);
	pc.numInstances = instanceCount;
	pc.gridCellSize = globalVoxelGridCellSize;
	pc.gridExtent = globalVoxelGridExtent;
	vkCmdPushConstants(cmdBuf, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VoxelIntegrationStage::PushConstants), &pc);
	vkCmdDispatch(cmdBuf, numWorkGroups, 1, 1);
}
