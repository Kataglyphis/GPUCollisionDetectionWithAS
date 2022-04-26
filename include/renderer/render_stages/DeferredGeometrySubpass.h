#pragma once
#include "Subpass.h"
#include <vector>
#include "RenderAttachmentInfo.h"

class DeferredGeometrySubpass : public Subpass
{
public:
	DeferredGeometrySubpass(RenderAttachmentInfo attachmentInfo);
	~DeferredGeometrySubpass();


	VkSubpassDependency getSubpassDependency(uint32_t subpassIndex);
	VkSubpassDescription getSubpassDescription();

private:
	
	std::vector<VkAttachmentReference> colorAttachmentReferenceGeometryPass;
	VkAttachmentReference depthAttachmentReference;
};

inline DeferredGeometrySubpass::DeferredGeometrySubpass(RenderAttachmentInfo attachmentInfo)
{
	// GBuffer
	colorAttachmentReferenceGeometryPass =
	{
		{ attachmentInfo.albedoIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
		{ attachmentInfo.normalIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
		{ attachmentInfo.materialIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
		{ attachmentInfo.positionIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
	};

	// Depth
	depthAttachmentReference =
	{ attachmentInfo.depthIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
}

inline DeferredGeometrySubpass::~DeferredGeometrySubpass()
{
}

inline VkSubpassDependency DeferredGeometrySubpass::getSubpassDependency(uint32_t subpassIndex)
{
	if (subpassIndex == 0) return getInitialSubpassDependency();
	VkSubpassDependency dependencyGeometryPass;
	dependencyGeometryPass.srcSubpass = subpassIndex-1;
	dependencyGeometryPass.dstSubpass = subpassIndex;
	dependencyGeometryPass.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencyGeometryPass.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencyGeometryPass.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencyGeometryPass.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencyGeometryPass.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependencyGeometryPass;
}

inline VkSubpassDescription DeferredGeometrySubpass::getSubpassDescription()
{
	VkSubpassDescription subpassDescriptionGeometryPass = {};
	subpassDescriptionGeometryPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptionGeometryPass.colorAttachmentCount = colorAttachmentReferenceGeometryPass.size();
	subpassDescriptionGeometryPass.pColorAttachments = colorAttachmentReferenceGeometryPass.data();
	subpassDescriptionGeometryPass.pDepthStencilAttachment = &depthAttachmentReference;
	return subpassDescriptionGeometryPass;
}
