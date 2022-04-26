#pragma once
#include "vulkan/vulkan.h"

class RenderAttachment
{
public:
	virtual ~RenderAttachment() {};
	virtual VkAttachmentDescription getAttachmentDescriptor() = 0;
	virtual VkImageView getImageView(uint32_t swapchainImageIndex) = 0;
	virtual VkClearValue getClearValue() = 0;
private:

};
