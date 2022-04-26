#pragma once
#include "Subpass.h"
#include <vector>
#include "RenderAttachmentInfo.h"

class PPLLRenderStageSubpass : public Subpass
{
public:
	PPLLRenderStageSubpass(RenderAttachmentInfo attachmentInfo);
	~PPLLRenderStageSubpass();


	VkSubpassDependency getSubpassDependency(uint32_t subpassIndex);
	VkSubpassDescription getSubpassDescription();

private:

	std::vector<VkAttachmentReference> inputReference;
	VkAttachmentReference colorAttachmentReference;
};

inline PPLLRenderStageSubpass::PPLLRenderStageSubpass(RenderAttachmentInfo attachmentInfo)
{
	// GBuffer
	inputReference =
	{
		// GBuffer
		{ attachmentInfo.albedoIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		{ attachmentInfo.normalIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		{ attachmentInfo.materialIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		// Position
		{ attachmentInfo.positionIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
	};

	colorAttachmentReference =
		// SwapchainColorBuffer
	{
		attachmentInfo.swapchainColorIndex,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
}

inline PPLLRenderStageSubpass::~PPLLRenderStageSubpass()
{
}

inline VkSubpassDependency PPLLRenderStageSubpass::getSubpassDependency(uint32_t subpassIndex)
{
	if (subpassIndex == 0) return getInitialSubpassDependency();
	VkSubpassDependency dependencyShadingPass = {};
	dependencyShadingPass.srcSubpass = subpassIndex - 1;
	dependencyShadingPass.dstSubpass = subpassIndex;
	dependencyShadingPass.srcStageMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	dependencyShadingPass.dstStageMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	dependencyShadingPass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencyShadingPass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencyShadingPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependencyShadingPass;
}

inline VkSubpassDescription PPLLRenderStageSubpass::getSubpassDescription()
{
	VkSubpassDescription subpassDescriptionShadingPass = {};
	subpassDescriptionShadingPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionShadingPass.inputAttachmentCount = 0;// inputReference.size();
	subpassDescriptionShadingPass.pInputAttachments = nullptr;// inputReference.data();
	subpassDescriptionShadingPass.colorAttachmentCount = 0;// 1;
	subpassDescriptionShadingPass.pColorAttachments = nullptr;//&colorAttachmentReference;
	return subpassDescriptionShadingPass;
}
