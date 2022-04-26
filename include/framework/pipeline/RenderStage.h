#pragma once
#include <vulkan/vulkan.h>

template<class T>
class RenderStage
{
public:
	~RenderStage() {};

	virtual void recordRenderCommands(VkCommandBuffer& cmdBuf, T params) = 0;

private:

};