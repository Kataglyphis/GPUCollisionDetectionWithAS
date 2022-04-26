#include "DepthAttachment.h"
#include "PipelineToolKit.h"

DepthAttachment::DepthAttachment(ResourceManager* rm)
{
	this->context = rm->context;
	VkImageCreateInfo imgCreateInfo = vk_default::renderTargetImageCreateInfo();
	imgCreateInfo.extent.width = context->width;
	imgCreateInfo.extent.height = context->height;
	imgCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	imgCreateInfo.format = format;


	createImage(rm->context->device, context->allocator, imgCreateInfo, image);

	VkImageViewCreateInfo imgViewCreateInfo = vk_default::renderTargetImageViewCreateInfo();
	imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imgViewCreateInfo.format = format;
	imgViewCreateInfo.image = image.image;

	createImageView(context->device, imgViewCreateInfo, image.view);
	rm->immediateImageLayoutTransition(image, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
}

DepthAttachment::~DepthAttachment()
{
	vkDestroyImageView(context->device, image.view, nullptr);
	destroyImage(context->device, context->allocator, image);
}

VkAttachmentDescription DepthAttachment::getAttachmentDescriptor()
{
	VkAttachmentDescription renderTargetAttchementDescription = vk_default::renderTargetAttchementDescription();
	renderTargetAttchementDescription.format = format;
	renderTargetAttchementDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	renderTargetAttchementDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;// VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	return renderTargetAttchementDescription;
}

VkImageView DepthAttachment::getImageView(uint32_t swapchainImageIndex)
{
	return image.view;
}

VkClearValue DepthAttachment::getClearValue()
{
	return clearValue;
}