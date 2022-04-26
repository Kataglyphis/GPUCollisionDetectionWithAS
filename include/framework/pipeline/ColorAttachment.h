#pragma once
#include "RenderAttachment.h"
#include "Context.h"

class ColorAttachment : public RenderAttachment
{
public:
	ColorAttachment(Context* pContext);
	VkImageView getImageView(uint32_t swapchainImageIndex);
	VkAttachmentDescription getAttachmentDescriptor();
	VkClearValue getClearValue();
	~ColorAttachment();
private:
	std::vector<VkImageView> imageViews;
	Context* context;
	VkFormat format;
	VkClearValue clearValue= {{ 205.0f/255.0f, 246.0f/255.0f, 255.0f/255.0f, 0.0f }};
};

