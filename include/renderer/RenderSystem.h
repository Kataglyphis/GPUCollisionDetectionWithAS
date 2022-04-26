#pragma once
#include "vulkan/vulkan.h"
#include "Context.h"
#include "ResourceManager.h"
#include "DeferredPipeline.h"
#include "DescriptorSet.h"
#include "Scene.h"
#include "Camera.h"
#include "SceneLoadInfo.h"
#include <DescriptorSetManager.h>
#include <render_stages/PathTraceRenderStage.h>
#include "DepthPeeling.h"



class RenderSystem
{
public:
	RenderSystem(ResourceManager* rm, Camera* camera, DescriptorSetManager* descriptorSetManager, Scene* scene);
	~RenderSystem();

	void init();
	void addStaticDescriptorWrites();
	void render();

	void reloadShaders();

private:
	ResourceManager* rm;
	Context* context;

	DeferredPipeline* deferredPipeline;
	PathTraceRenderStage* ptPipeline;
	DepthPeeling* depthPeeling;

	Scene* scene;

	Camera* camera;

	DescriptorSetManager* descriptorSetManager;

	DescriptorSet* descriptorSetGeometryPass;
	DescriptorSet* descriptorSetShadingPass;
	DescriptorSet* descriptorSetParticlePass;
	DescriptorSet* descriptorSetPathTracing;
	DescriptorSet* descriptorSetDepthPeelingPass;

	bool init_depth_peeling = false;

	VkCommandBuffer drawCmdBuf;

	void createDrawCmdBuf();
	void update();
	void createSynchronizationObjects();
	void draw();

};

