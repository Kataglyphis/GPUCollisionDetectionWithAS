#pragma once

#include <vector>
#include <map>
#include <vulkan/vulkan.h>
#include "DescriptorSetLayoutDefinitions.h"
#include "Resources.h"

struct DescriptorSetUpdate
{
	uint32_t								dstBinding;
	uint32_t								descriptorCount;
	VkDescriptorType						descriptorType;
	std::vector<VkDescriptorImageInfo>		imageInfo;
	std::vector<VkDescriptorBufferInfo>		bufferInfo;
	VkAccelerationStructureKHR				accelerationStructure;
};

class DescriptorSet
{
public:
	DescriptorSet(VkDevice device, descriptor_set_layout_definition_t layoutDefinition);
	~DescriptorSet();

	std::vector<VkDescriptorPoolSize> getPoolSizes(uint32_t setCount);
	void createSets(VkDevice device, uint32_t count, VkDescriptorPool pool);
	void addUpdate(DescriptorSetUpdate update, uint32_t setIndex);
	void addUpdate(Buffer buffer, uint32_t setIndex, uint32_t binding);
	void addUpdate(std::vector<Image> images, uint32_t setIndex, uint32_t binding);
	void addUpdate(VkSampler sampler, uint32_t setIndex, uint32_t binding);
	void addUpdate(VkAccelerationStructureKHR as, uint32_t setIndex, uint32_t binding);

	void processUpdates(VkDevice device, uint32_t setIndex);
	void createLayout(VkDevice device);
	std::vector<VkDescriptorSet>							set;
	VkDescriptorSetLayout									layout;
private:

	void bind(uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags stage);
	void bind(descriptor_set_layout_definition_t layoutDefinition);

	std::vector<std::map<uint32_t, DescriptorSetUpdate>>	updates;
	std::map<uint32_t, VkDescriptorSetLayoutBinding>		bindings;
};

