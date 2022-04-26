#pragma once

#include "DescriptorSet.h"
#include "Subpass.h"
#include "RenderAttachment.h"
#include "RenderAttachmentInfo.h"
#include "RenderStage.h"

//struct RenderParams {};
template<class T>
class RasterizationStage : public RenderStage<T>
{
public:
	~RasterizationStage() {};

	virtual void destroyPipeline() = 0;
	virtual void createPipeline(VkRenderPass renderPass) = 0;
	virtual void recordRenderCommands(VkCommandBuffer& cmdBuf, T params) = 0;
	virtual Subpass* getSubpass() = 0;
private:

};
