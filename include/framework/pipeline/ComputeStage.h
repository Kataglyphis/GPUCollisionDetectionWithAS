#pragma once


#include "RenderStage.h"

template <class T>
class ComputeStage : public RenderStage<T>
{
public:
	~ComputeStage() {};
	virtual void destroyPipeline() = 0;
	virtual void createPipeline() = 0;
	virtual void recordRenderCommands(VkCommandBuffer& cmdBuf, T params) = 0;

private:

};
