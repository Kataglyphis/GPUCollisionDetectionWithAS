#pragma once
#include <vulkan/vulkan.h>

#include "ColorAttachment.h"
#include "DepthAttachment.h"
#include "InputAttachment.h"

#include "DrawParams.h"

#include "DeferredGeometryRenderStage.h"
#include "DeferredShadingRenderStage.h"
#include "ParticleRenderStage.h"
#include "ImGuiRenderStage.h"



struct DeferredShaders
{
	std::string		geometryVert = "render_geometry.vert";
	std::string		geometryFrag = "render_geometry.frag";
	std::string		shadingVert  = "render_shading.vert";
	std::string		shadingFrag  = "render_shading.frag";

	std::string		particleVert = "render_particle.vert";
	std::string		particleFrag = "render_particle.frag";
	std::string		particleGeom = "render_particle.geom";
};




class DeferredPipeline
{
public:
	DeferredPipeline();
	DeferredPipeline(ResourceManager* rm, DescriptorSet* descriptorSetDeferredGeometry, DescriptorSet* descriptorSetDeferredShading, DescriptorSet* descriptorSetDeferredParticle);
	void reloadShaders();
	void init();
	void recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams drawParams);
	~DeferredPipeline();
	std::vector<RenderAttachment*> attachments;
	RenderAttachmentInfo attachmentInfo;
	VkRenderPass				renderPassDeferred;
	std::vector<VkFramebuffer>	framebuffers;
private:

	void createRenderAttachments();
	void createRenderStages();
	void createPipelines();
	void createFramebuffers();
	void createRenderPass();




	// Renderstages
	DeferredGeometryRenderStage*	geometryStage;
	DeferredShadingRenderStage*		shadingStage;
	ParticleRenderStage*			particleStage;
	ImGuiRenderStage*				imGuiStage;

	std::vector<RasterizationStage<DrawParams>*>		renderStages;

	DescriptorSet* descSetGeometry;
	DescriptorSet* descSetShading;
	DescriptorSet* descSetParticle;
	ResourceManager* rm;
	Context* context;

	DeferredShaders			shaders;

	VkShaderModule			shaderModuleDeferredGeometryVert;
	VkShaderModule			shaderModuleDeferredGeometryFrag;

	VkShaderModule			shaderModuleDeferredShadingVert;
	VkShaderModule			shaderModuleDeferredShadingFrag;

	VkShaderModule			shaderModuleDeferredParticleVert;
	VkShaderModule			shaderModuleDeferredParticleFrag;
	VkShaderModule			shaderModuleDeferredParticleGeom;
};
