#include "InputAttachment.h"

#include "Utilities.h"
#include "PipelineToolKit.h"


InputAttachment::InputAttachment(Context* pContext, VkFormat format)
{
	this->format = format;
	this->context = pContext;
	VkImageCreateInfo imgCreateInfo = vk_default::renderTargetImageCreateInfo();
	imgCreateInfo.extent.width = context->width;
	imgCreateInfo.extent.height = context->height;
	imgCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	VkImageViewCreateInfo imgViewCreateInfo = vk_default::renderTargetImageViewCreateInfo();
	imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;


	imgCreateInfo.format = format;
	createImage(context->device, context->allocator, imgCreateInfo, image);
	imgViewCreateInfo.format = format;
	imgViewCreateInfo.image = image.image;
	createImageView(context->device, imgViewCreateInfo, image.view);
}

VkImageView InputAttachment::getImageView(uint32_t swapchainImageIndex)
{
	return image.view;
}


VkAttachmentDescription InputAttachment::getAttachmentDescriptor()
{

	VkAttachmentDescription renderTargetAttchementDescription = vk_default::renderTargetAttchementDescription();
	renderTargetAttchementDescription.format = format;
	return renderTargetAttchementDescription;
}




InputAttachment::~InputAttachment()
{
	vkDestroyImageView(context->device, image.view, nullptr);
	destroyImage(context->device, context->allocator, image);
}

VkClearValue InputAttachment::getClearValue()
{
	return clearValue;
}