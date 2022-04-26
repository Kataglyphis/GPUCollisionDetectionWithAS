#include "CollisionComputeStage.h"
#include "Utilities.h"


CollisionComputeStage::CollisionComputeStage(Context* context, DescriptorSet* descriptorSet)
{
	this->descriptorSet = descriptorSet;
	this->context = context;
	createPipeline();
}

CollisionComputeStage::~CollisionComputeStage()
{
}

void CollisionComputeStage::createPipeline()
{
	compileShaders();

	std::vector<VkSpecializationMapEntry> specEntries;
	VkSpecializationMapEntry specEntry;
	specEntry.constantID = 0;
	specEntry.offset = 0;
	specEntry.size = sizeof(uint32_t);
	specEntries.push_back(specEntry);

	std::vector<uint32_t> specValues = { workGroupSizeX };

	VkSpecializationInfo specInfo;
	specInfo.mapEntryCount = specEntries.size();
	specInfo.pMapEntries = specEntries.data();
	specInfo.dataSize = specValues.size() * sizeof(uint32_t);
	specInfo.pData = specValues.data();

	VkPushConstantRange pushConstantRange;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(CollisionComputeStage::PushConstants);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSet->layout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	VkResult result = vkCreatePipelineLayout(context->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	ASSERT_VULKAN(result);

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStageCreateInfo.module = shaderModule;
	shaderStageCreateInfo.pName = "main";
	shaderStageCreateInfo.pSpecializationInfo = &specInfo;

	VkComputePipelineCreateInfo pipelineCreateInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
	pipelineCreateInfo.stage = shaderStageCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	result = vkCreateComputePipelines(context->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
	ASSERT_VULKAN(result);
}

void CollisionComputeStage::compileShaders()
{
	compileShader(context->device, shader, &shaderModule);
}



void CollisionComputeStage::recordRenderCommands(VkCommandBuffer& cmdBuf, ComputeParams params)
{
	uint32_t instanceCount = params.geom->instances.size();
	uint32_t vertexCount = params.geom->numVertices;
	uint32_t numWorkGroups = (instanceCount * vertexCount + workGroupSizeX - 1) / workGroupSizeX;

	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
		&descriptorSet->set[context->currentSwapchainIndex], 0, nullptr);

	CollisionComputeStage::PushConstants pc{};
	pc.numVertecies = vertexCount;
	pc.vertexAddress = getBufferAddress(context->device, params.geom->vertexBuffer.buffer);
	pc.centerOfMass = glm::vec4(params.geom->model->centerOfMass,0.0);
	pc.numInstances = instanceCount;
	pc.customIntersectionIndexOffset = params.geom->instances[0].customIntersectionIndex; // Initial offset
	vkCmdPushConstants(cmdBuf, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0,sizeof(CollisionComputeStage::PushConstants), &pc);
	vkCmdDispatch(cmdBuf, numWorkGroups, 1, 1);
}

void CollisionComputeStage::destroyPipeline()
{
	vkDestroyPipelineLayout(context->device, pipelineLayout, nullptr);
	vkDestroyPipeline(context->device, pipeline, nullptr);
}