#pragma once
#include "RenderAttachment.h"
#include "Context.h"

class InputAttachment : public RenderAttachment
{
public:
	~InputAttachment();

	InputAttachment(Context* pContext, VkFormat format);

	VkImageView getImageView(uint32_t swapchainImageIndex);
	VkAttachmentDescription getAttachmentDescriptor();
	VkClearValue getClearValue();
private:
	Context* context;

	VkFormat format; //{ VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R16G16B16A16_SFLOAT };
	Image image;
	VkClearValue clearValue = { { 0.0f, 0.0f, 0.0f, 0.0f } };
};

