#pragma once

#include "RasterizationStage.h"
#include "Context.h"
#include "DrawParams.h"

class ParticleRenderStage : public RasterizationStage<DrawParams>
{
public:
	ParticleRenderStage(Context* context, DescriptorSet* descriptorSet, RenderAttachmentInfo attachementInfo, uint32_t subpassIndex);
	void destroyPipeline();
	~ParticleRenderStage();
	void destroyShaders();
	void compileShaders();
	void createPipeline(VkRenderPass renderPass);
	void recordRenderCommands(VkCommandBuffer& cmdBuf, VkBuffer* pParticleBuffer, uint32_t particleCount);
	void recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params);
	Subpass* getSubpass();
private:

	Subpass* subpass;
	Context* context;
	DescriptorSet* descriptorSet;

	struct Shaders
	{
		std::string		vert = "render_particle.vert";
		std::string		geom = "render_particle.geom";
		std::string		frag = "render_particle.frag";
	};

	Shaders					shaders;

	VkShaderModule			shaderModuleVert;
	VkShaderModule			shaderModuleGeom;
	VkShaderModule			shaderModuleFrag;

	VkPipeline				pipeline;
	VkPipelineLayout		pipelineLayout;

	uint32_t subpassIndex;
};
