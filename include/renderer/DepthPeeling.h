#pragma once

#include <vulkan/vulkan.h>
#include "DrawParams.h"
#include "InputAttachment.h"

#include <string>
#include <Context.h>
#include <ResourceManager.h>
#include <DepthAttachment.h>
#include <ColorAttachment.h>
#include <PPLLRenderStage.h>
#include <DepthPeelingSortAndFill.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <DescriptorSetManager.h>


#include <stdint.h>

class DepthPeeling
{
public:
	DepthPeeling();
	DepthPeeling(ResourceManager* rm, DescriptorSetManager* descriptorSetManager);
	void reloadShaders();

	Buffer* get_particle_representation_buffer();
	Buffer* get_counter_buffer();
	Buffer* get_node_buffer();

	void init();
	void recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams drawParams);
	~DepthPeeling();

	std::vector<RenderAttachment*>				attachments;
	RenderAttachmentInfo						attachmentInfo;
	VkRenderPass								renderPassPPLLDepthPeeling;
	VkRenderPass								renderPassSortAndLinkDepthPeeling;
	std::vector<VkFramebuffer>					framebuffers;
	std::vector<VkFramebuffer>					framebuffers_second_pass;

private:

	void createRenderAttachments();
	void createRenderStages();
	void createPipelines();
	void createRenderPass();
	void createFramebuffers();

	void createAndUploadBuffersAndImages();

	DescriptorSetManager* descriptorSetManager;

	// render stages
	PPLLRenderStage*			ppllDepthPeelingRenderStage;
	DepthPeelingSortAndFill*	sortAndFillDepthPeelingRenderStage;

	std::vector<RasterizationStage<DrawParams>*>		renderStages;

	Context* context;
	ResourceManager* rm;

	DescriptorSet* descriptorSetDepthPeelingPass;

	VkShaderModule shaderModuleDepthPeelingVert;
	VkShaderModule shaderModuleDepthPeelingFrag;

	std::vector<Node> nodes;
	std::vector<Counter> counters;
	std::vector<DepthPeelingConstants> depthPeelingVector;
	std::vector<Particle> objectParticles;

	Buffer linked_list;
	Buffer counter_buffer;
	Buffer depthPeelingConstantsBuf;
	Buffer particleBuf;

	VkCommandBuffer uploadCmdBuf;

	std::vector<Image> head_pointer;
};

