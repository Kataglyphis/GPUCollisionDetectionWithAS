#pragma once
#pragma once
#include "Subpass.h"
#include <vector>
#include "RenderAttachmentInfo.h"

class ImGuiSubpass : public Subpass
{
public:
	ImGuiSubpass(RenderAttachmentInfo attachmentInfo);
	~ImGuiSubpass();

	VkSubpassDependency getSubpassDependency(uint32_t subpassIndex);
	VkSubpassDescription getSubpassDescription();

private:
	VkAttachmentReference colorAttachmentReferenceShadingPass;
};

inline ImGuiSubpass::ImGuiSubpass(RenderAttachmentInfo attachmentInfo)
{
	colorAttachmentReferenceShadingPass =
		// SwapchainColorBuffer
	{
		attachmentInfo.swapchainColorIndex,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
}

inline ImGuiSubpass::~ImGuiSubpass()
{
}

inline VkSubpassDependency ImGuiSubpass::getSubpassDependency(uint32_t subpassIndex)
{
	if (subpassIndex == 0) return getInitialSubpassDependency();
	VkSubpassDependency dependencyShadingPass = {};
	dependencyShadingPass.srcSubpass = subpassIndex - 1;
	dependencyShadingPass.dstSubpass = subpassIndex;
	dependencyShadingPass.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencyShadingPass.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencyShadingPass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencyShadingPass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencyShadingPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependencyShadingPass;
}

inline VkSubpassDescription ImGuiSubpass::getSubpassDescription()
{
	VkSubpassDescription subpassDescriptionShadingPass = {};
	subpassDescriptionShadingPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionShadingPass.colorAttachmentCount = 1;
	subpassDescriptionShadingPass.pColorAttachments = &colorAttachmentReferenceShadingPass;
	return subpassDescriptionShadingPass;
}
