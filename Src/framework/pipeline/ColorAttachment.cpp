#include "ColorAttachment.h"
#include "PipelineToolKit.h"

ColorAttachment::ColorAttachment(Context* pContext)
{
	context = pContext;
	format = context->swapChainImageFormat;
	for (uint32_t i = 0; i < context->swapChainImages.size(); i++)
	{
		imageViews.push_back(context->swapChainImages[i].image_view);
	}
}

VkImageView ColorAttachment::getImageView(uint32_t swapchainImageIndex)
{
	return imageViews[swapchainImageIndex];
}

VkAttachmentDescription ColorAttachment::getAttachmentDescriptor()
{
	VkAttachmentDescription attachmentDescriptionSwapChainColor = vk_default::renderTargetAttchementDescription();
//#ifdef 0
//	attachmentDescriptionSwapChainColor.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
//	attachmentDescriptionSwapChainColor.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
//#endif

	attachmentDescriptionSwapChainColor.format = format;
	attachmentDescriptionSwapChainColor.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	return attachmentDescriptionSwapChainColor;
}

VkClearValue ColorAttachment::getClearValue()
{
	return clearValue;
}


ColorAttachment::~ColorAttachment()
{
}