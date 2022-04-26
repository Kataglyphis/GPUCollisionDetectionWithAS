#pragma once
#include "Resources.h"
#include "Model.h"
#include "ResourceManager.h"
#include "DrawInst.h"
#include <DrawSurf.h>


#define GRID_SIZE_MAX 50
#define MAX_INSTANCES GRID_SIZE_MAX*GRID_SIZE_MAX*GRID_SIZE_MAX

class DrawGeom
{
public:
	DrawGeom();
	void recordUpdateBLAS(VkCommandBuffer& cmdBuf, Buffer& stagingBuffer);
	void createBLAS();
	std::vector<VkAccelerationStructureGeometryKHR> getGeometryKHR();
	std::vector <VkAccelerationStructureBuildRangeInfoKHR> getBuildRangeInfo();
	~DrawGeom();


	DrawGeom(ResourceManager* rm, Model* model, uint32_t maxInstances);

	static DrawGeom* getDefaultCube(ResourceManager* rm);
	void getDefaultCubeVoxelGrid(ResourceManager* rm);
	void addInstance(ResourceManager* rm, InstanceShaderData shaderData);
	void addInstances(ResourceManager* rm, std::vector<DrawInst> addInstances);
	void createInstanceBuffer(uint32_t maxInstances);
	void createIndirectDrawBuffer(ResourceManager* rm);
	void updateInstances();
	static const VkBuildAccelerationStructureFlagsKHR BLAS_BUILD_FLAGS = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;


	Context* context;

	Buffer vertexBuffer;
	Buffer indexBuffer;
	Buffer instanceBuffer;
	Buffer materialBuffer;
	Buffer indirectDrawBuffer;

	std::vector<DrawSurf> surfs;


	VkDeviceAddress instanceBufferAddress;
	VkDeviceAddress materialBufferAddress;


	uint32_t numVertices;
	uint32_t numIndices;

	// May be empty
	Model* model;
	// Bottom level as
	BLAS blas{};
	// List of instances
	std::vector<DrawInst> instances{};

	// For classic collision detection
	Buffer voxelGridBuffer;
	VkDeviceAddress voxelGridBufferAddress;
	uint32_t numVoxels;

	// Status
	bool hasBlas				= false;
	bool hasInstanceBuffer		= false;
	bool hasIndirectDrawBuffer	= false;
	bool hasVoxelGrid			= false;

	//DELETE_COPY_CONSTRUCTORS(DrawGeom)

private:

};




