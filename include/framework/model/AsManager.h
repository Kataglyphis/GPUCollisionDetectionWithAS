#pragma once
#include "Resources.h"
#include "ResourceManager.h"
#include "DrawInst.h"

class AsManager
{
public:
	AsManager(ResourceManager* rm);
	~AsManager();

	void uploadInstanceBuffers(std::vector<DrawInst*> inst, bool recreate);
	void buildTLAS();

	//void buildTLAS(std::vector<DrawInst*> inst);

	static const VkBuildAccelerationStructureFlagsKHR TLAS_BUILD_FLAGS = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;


	VkAccelerationStructureKHR tlas;
	Buffer instanceDescriptorBuffer;
	Buffer instanceBufferKHR;
	uint32_t instanceCount;

private:
	ResourceManager*	rm;
	Context*			context;
};

