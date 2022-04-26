#pragma once
#include <vulkan/vulkan.h>

class Subpass
{
public:
	virtual ~Subpass() {};

	virtual VkSubpassDependency getSubpassDependency(uint32_t subpassIndex) = 0;
	virtual VkSubpassDescription getSubpassDescription() = 0;
	static VkSubpassDependency getFinalSubpassDependency();
	static VkSubpassDependency getInitialSubpassDependency();
private:

};

inline VkSubpassDependency Subpass::getFinalSubpassDependency()
{
	VkSubpassDependency finalSubpassDependency;
	finalSubpassDependency.srcSubpass = 0;
	finalSubpassDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
	finalSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	finalSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	finalSubpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	finalSubpassDependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	finalSubpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return finalSubpassDependency;
}

inline VkSubpassDependency Subpass::getInitialSubpassDependency()
{
	VkSubpassDependency initialSubpassDependency = {};
	initialSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	initialSubpassDependency.dstSubpass = 0;
	initialSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	initialSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	initialSubpassDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	initialSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	initialSubpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	return initialSubpassDependency;
}
