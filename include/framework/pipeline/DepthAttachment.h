#pragma once

#include "RenderAttachment.h"
#include "ResourceManager.h"

class DepthAttachment : public RenderAttachment
{
public:
	DepthAttachment(ResourceManager* rm);
	~DepthAttachment();

	VkAttachmentDescription getAttachmentDescriptor();

	VkImageView getImageView(uint32_t swapchainImageIndex);

	VkClearValue getClearValue();

private:

	Image image;
	Context* context;
	const VkFormat format = VK_FORMAT_D32_SFLOAT;
	VkClearValue clearValue = { { 1.0f, 0 } };
};

