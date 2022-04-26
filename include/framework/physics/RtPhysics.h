#pragma once
#include "ResourceManager.h"
#include "DescriptorSetManager.h"
#include "Scene.h"
#include "ModelMatrixComputeStage.h"
#include <CollisionComputeStage.h>

class RtPhysics
{
public:
	RtPhysics(ResourceManager* rm, DescriptorSetManager* descriptorSetManager);

	void setUpTimeQuerys();

	void runSimulation(Scene* scene);

	void createSimulationCommandBuffer(Scene* scene);

	void createBuffers();

	void reloadShaders();

	~RtPhysics();

private:
	ResourceManager* rm;
	Context* context;
	DescriptorSetManager* descManager;
	DescriptorSet* descriptorSetIntergration;
	DescriptorSet* descriptorSetCollision;
	ModelMatrixComputeStage* modelMatrixComputeStage;
	CollisionComputeStage* collisionComputeStage;
	VkCommandBuffer simulationCmdBuf;
	Buffer instanceStorageBuffer;
	VkQueryPool queryPool;
	uint64_t queryResults[2];
};

