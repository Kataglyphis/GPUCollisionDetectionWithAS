#pragma once
#include "RasterizationStage.h"
#include "Context.h"
#include "DrawParams.h"
#include "ResourceManager.h"



class DeferredGeometryRenderStage : public RasterizationStage<DrawParams>
{
public:

	struct PushConstants
	{
		VkDeviceAddress instanceBufferAddress;	//64 bit
		VkDeviceAddress maetrialBufferAddress;	//64 bit
	};

	DeferredGeometryRenderStage(ResourceManager* rm, DescriptorSet* descriptorSet, RenderAttachmentInfo attachementInfo, uint32_t subpassIndex);
	void destroyPipeline();
	~DeferredGeometryRenderStage();
	void compileShaders();
	void destroyShaders();
	void createPipeline(VkRenderPass renderPass);
	void recordRenderCommands(VkCommandBuffer& cmdBuf, VkBuffer* pVertexBuffer, VkBuffer indexBuffer, uint32_t indexCount);
	void recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params);
	Subpass* getSubpass();
private:
	
	ResourceManager*	 rm;
	Subpass*		subpass;
	Context*		context;
	DescriptorSet*	descriptorSet;
	uint32_t		subpassIndex;

	struct Shaders
	{
		std::string		vert = "render_geometry.vert";
		std::string		frag = "render_geometry.frag";
	};

	Shaders					shaders;

	VkShaderModule			shaderModuleVert;
	VkShaderModule			shaderModuleFrag;

	VkPipeline				pipeline;
	VkPipelineLayout		pipelineLayout;
};

