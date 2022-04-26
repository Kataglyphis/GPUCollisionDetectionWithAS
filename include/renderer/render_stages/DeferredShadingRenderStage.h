#pragma once
#include "RasterizationStage.h"
#include "Context.h";
#include "DrawParams.h"

class DeferredShadingRenderStage : public RasterizationStage<DrawParams>
{
public:
	struct PushConstants
	{
		uint32_t enablePathTracing;
		uint32_t placeholder;
	};

	DeferredShadingRenderStage(Context* context, DescriptorSet* descriptorSet, RenderAttachmentInfo attachementInfo, uint32_t subpassIndex);
	void destroyPipeline();
	~DeferredShadingRenderStage();
	void destroyShaders();
	void compileShaders();
	void createPipeline(VkRenderPass renderPass);
	void recordRenderCommands(VkCommandBuffer& cmdBuf);
	void recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params);
	Subpass* getSubpass();
private:
	Subpass* subpass;
	struct Shaders
	{
		std::string		vert = "render_shading.vert";
		std::string		frag = "render_shading.frag";
	};
	Shaders					shaders;

	Context* context;
	DescriptorSet* descriptorSet;

	VkShaderModule			shaderModuleVert;
	VkShaderModule			shaderModuleFrag;

	VkPipeline				pipeline;
	VkPipelineLayout		pipelineLayout;

	uint32_t subpassIndex;

	PushConstants pc;
};
