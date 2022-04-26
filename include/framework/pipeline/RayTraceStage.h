#pragma once
#include <vulkan/vulkan.h>
#include "RenderStage.h"
#include "DrawInst.h"
#include <vector>

template <class T>
class RayTraceStage : public RenderStage<T>
{
public:
	~RayTraceStage() {};
	virtual void destroyPipeline() = 0;
	virtual void createPipeline() = 0;
	virtual void recordRenderCommands(VkCommandBuffer& cmdBuf, T params) = 0;
	//virtual void buildShaderBindingTables(std::vector<DrawInst*> inst) = 0;
private:

};
