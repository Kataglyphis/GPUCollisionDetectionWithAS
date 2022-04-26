#pragma once
#include "Subpass.h"
#include <vector>
#include "RenderAttachmentInfo.h"

class DeferredShadingSubpass : public Subpass
{
public:
	DeferredShadingSubpass(RenderAttachmentInfo attachmentInfo);
	~DeferredShadingSubpass();

	VkSubpassDependency getSubpassDependency(uint32_t subpassIndex);
	VkSubpassDescription getSubpassDescription();

private:
	std::vector<VkAttachmentReference> inputReference;
	VkAttachmentReference colorAttachmentReferenceShadingPass;
};

inline DeferredShadingSubpass::DeferredShadingSubpass(RenderAttachmentInfo attachmentInfo)
{
	inputReference =
	{
		// GBuffer
		{ attachmentInfo.albedoIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		{ attachmentInfo.normalIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		{ attachmentInfo.materialIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		// Position
		{ attachmentInfo.positionIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
	};

	colorAttachmentReferenceShadingPass =
		// SwapchainColorBuffer
	{
		attachmentInfo.swapchainColorIndex,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
}

inline DeferredShadingSubpass::~DeferredShadingSubpass()
{
}

inline VkSubpassDependency DeferredShadingSubpass::getSubpassDependency(uint32_t subpassIndex)
{
	if (subpassIndex == 0) return getInitialSubpassDependency();
	VkSubpassDependency dependencyShadingPass = {};
	dependencyShadingPass.srcSubpass = subpassIndex-1;
	dependencyShadingPass.dstSubpass = subpassIndex;
	dependencyShadingPass.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencyShadingPass.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencyShadingPass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencyShadingPass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencyShadingPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependencyShadingPass;
}

inline VkSubpassDescription DeferredShadingSubpass::getSubpassDescription()
{
	VkSubpassDescription subpassDescriptionShadingPass = {};
	subpassDescriptionShadingPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionShadingPass.inputAttachmentCount = inputReference.size();
	subpassDescriptionShadingPass.pInputAttachments = inputReference.data();
	subpassDescriptionShadingPass.colorAttachmentCount = 1;
	subpassDescriptionShadingPass.pColorAttachments = &colorAttachmentReferenceShadingPass;
	return subpassDescriptionShadingPass;
}
