#pragma once
#pragma once
#include "Subpass.h"
#include <vector>
#include "RenderAttachmentInfo.h"

class DeferredParticleSubpass : public Subpass
{
public:
	DeferredParticleSubpass(RenderAttachmentInfo attachmentInfo);
	~DeferredParticleSubpass();


	VkSubpassDependency getSubpassDependency(uint32_t subpassIndex);
	VkSubpassDescription getSubpassDescription();

private:

	std::vector<VkAttachmentReference> colorAttachmentReference;
	VkAttachmentReference depthAttachmentReference;
};

inline DeferredParticleSubpass::DeferredParticleSubpass(RenderAttachmentInfo attachmentInfo)
{
	// GBuffer
	colorAttachmentReference =
	{
		{ attachmentInfo.swapchainColorIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
	};

	// Depth
	depthAttachmentReference =
	{ attachmentInfo.depthIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
}

inline DeferredParticleSubpass::~DeferredParticleSubpass()
{
}

inline VkSubpassDependency DeferredParticleSubpass::getSubpassDependency(uint32_t subpassIndex)
{
	if (subpassIndex == 0) return getInitialSubpassDependency();
	VkSubpassDependency dependencyParticlePass;
	dependencyParticlePass.srcSubpass = subpassIndex - 1;
	dependencyParticlePass.dstSubpass = subpassIndex;
	dependencyParticlePass.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencyParticlePass.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencyParticlePass.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencyParticlePass.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencyParticlePass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependencyParticlePass;
}

inline VkSubpassDescription DeferredParticleSubpass::getSubpassDescription()
{
	VkSubpassDescription subpassDescriptionPariclePass = {};
	subpassDescriptionPariclePass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionPariclePass.colorAttachmentCount = colorAttachmentReference.size();
	subpassDescriptionPariclePass.pColorAttachments = colorAttachmentReference.data();
	subpassDescriptionPariclePass.pDepthStencilAttachment = &depthAttachmentReference;
	return subpassDescriptionPariclePass;
}
