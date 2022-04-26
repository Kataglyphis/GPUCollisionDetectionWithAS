#pragma once
#include "glm/glm.hpp"
#include "Model.h"
#include "DrawGeom.h"
#include "ShaderStructs.h"
#include "DescriptorSet.h"
#include "ObjLoader.h"
#include "Camera.h"
#include "DrawInst.h"
#include "AsManager.h"
#include "SceneLoadInfo.h"
#include <DescriptorSetManager.h>
#include <DepthPeeling.h>

struct SceneView
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 camPos;
};

//struct SceneGeometry
//{
//	glm::mat4 modelMatrix;
//	DrawGeom geom;
//};




class Scene
{
public:
	~Scene();
	std::vector<DrawGeom*> geom;
	std::vector<DrawInst*> inst;

	SceneView view;

	Buffer perFrameConstantsBuf;

	VkCommandBuffer uploadCmdBuf;

	void createInstanceGrid(DrawGeom* geom, float scaleFactor, float distance);

	void resetDynamicGeometry();



	ObjLoader*				objLoader;
	MaterialManager*		matManager;
	MaterialTextureManager* texManager;
	MaterialTextureManager* particleTexManager;
	bool asInstancesInitialized = false;
	

	DepthPeeling* depthPeeling;


	Scene(ResourceManager* rm, DescriptorSetManager* descriptorSetManager);
	void updateInstances();
	void update(Camera* camera);
	void load(SceneLoadInfo loadInfo);
	//void update();
private:
	void updatePerFrameConstants();
	ResourceManager* rm;
	DescriptorSetManager* descSetManager;
	Context* context;
	Buffer materialBuffer;
	std::vector<Image> textures;
	std::vector<Image> particleTextures;
	VkSampler sampler;
	VkSampler particleSampler;
};