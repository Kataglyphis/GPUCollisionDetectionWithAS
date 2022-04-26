#pragma once
#pragma once
#include "RasterizationStage.h"
#include "Context.h";
#include "DrawParams.h"
#include <ResourceManager.h>

class ImGuiRenderStage : public RasterizationStage<DrawParams>
{
public:
	ImGuiRenderStage(ResourceManager* rm, DescriptorSet* descriptorSet, RenderAttachmentInfo attachementInfo, uint32_t subpassIndex);
	void destroyPipeline();
	~ImGuiRenderStage();
	void destroyShaders();
	void compileShaders();
	void createPipeline(VkRenderPass renderPass);
	void addGVars(GVar_Cat category);
	void render_gui();
	void recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params);
	Subpass* getSubpass();

private:
	Subpass*			subpass;
	Context*			context;
	ResourceManager*	rm;
	VkDescriptorPool	gui_descriptor_pool;
	bool				isInit;
	uint32_t			subpassIndex;

	bool newWorkgroupSizesAreSuitable(int workGroupSizeX, int workGroupSizeY, int workGroupSizeZ);
	bool newNumParticlesAreSuitable(int numberParticlesX, int numberParticlesY, int numberParticlesZ);
	void addGVar(GVar* gv);
};
