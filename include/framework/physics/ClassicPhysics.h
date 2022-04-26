#pragma once
#include "ResourceManager.h"
#include "DescriptorSetManager.h"
#include "Scene.h"
#include <GlobalVoxelGridGenStage.h>
#include <VoxelIntegrationStage.h>
#include "ModelMatrixComputeStage.h"
#include "ShaderStructs.h"



class ClassicPhysics
{
public:
	ClassicPhysics(ResourceManager* rm, DescriptorSetManager* descriptorSetManager);
	~ClassicPhysics();

	void setUpTimeQuerys();

	void runSimulation(Scene* scene);

	void createSimulationCommandBuffer(Scene* scene);

	void createBuffers();

	void reloadShaders();

	ResourceManager* rm;
	Context* context;
	DescriptorSetManager* descManager;
	GlobalVoxelGridGenStage* voxelGenStage;
	VoxelIntegrationStage* voxelIntegrationStage;
	ModelMatrixComputeStage* modelMatrixComputeStage;
	VkCommandBuffer simulationCmdBuf;
	Buffer globalVoxelGridBuffer;
	Buffer instanceStorageBuffer;
	VkQueryPool queryPool;
	uint64_t queryResults[2];

private:

	
};
