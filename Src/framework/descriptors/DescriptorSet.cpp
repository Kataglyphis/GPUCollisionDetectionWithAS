#include "DescriptorSet.h"
#include "Utilities.h"

DescriptorSet::DescriptorSet(VkDevice device, descriptor_set_layout_definition_t layoutDefinition)
{
	bind(layoutDefinition);
	createLayout(device);
	updates.resize(MAX_FRAME_DRAWS);
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::bind(descriptor_set_layout_definition_t layoutDefinition)
{
	for (VkDescriptorSetLayoutBinding layoutBinding : layoutDefinition)
	{
		bindings[layoutBinding.binding] = layoutBinding;
	}
}

void DescriptorSet::bind(uint32_t binding, VkDescriptorType type, uint32_t count, VkShaderStageFlags stage)
{
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = type;
	layoutBinding.descriptorCount = count;
	layoutBinding.stageFlags = stage;

	bindings[binding] = layoutBinding;
}

void DescriptorSet::addUpdate(DescriptorSetUpdate update, uint32_t setIndex)
{
	updates[setIndex][update.dstBinding] = update;
}


void DescriptorSet::addUpdate(Buffer buffer, uint32_t setIndex , uint32_t binding)
{
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = buffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkDescriptorSetLayoutBinding layout = bindings[binding];

	DescriptorSetUpdate update = {};
	update.dstBinding = layout.binding;
	update.descriptorCount = layout.descriptorCount;
	update.descriptorType = layout.descriptorType;
	update.bufferInfo = { bufferInfo };

	addUpdate(update, setIndex);
}

void DescriptorSet::addUpdate(std::vector<Image> images, uint32_t setIndex, uint32_t binding)
{
	std::vector<VkDescriptorImageInfo> imageInfos;
	for (uint32_t i = 0; i < images.size(); i++)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageView = images[i].view;
		imageInfo.imageLayout = images[i].layout;
		imageInfos.push_back(imageInfo);
	}

	VkDescriptorSetLayoutBinding layout = bindings[binding];

	DescriptorSetUpdate update = {};
	update.dstBinding = layout.binding;
	update.descriptorCount = imageInfos.size();
	update.descriptorType = layout.descriptorType;
	update.imageInfo = imageInfos;

	addUpdate(update, setIndex);
}

void DescriptorSet::addUpdate(VkSampler sampler, uint32_t setIndex, uint32_t binding)
{
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.sampler = sampler;

	VkDescriptorSetLayoutBinding layout = bindings[binding];
	DescriptorSetUpdate update = {};
	update.dstBinding = layout.binding;
	update.descriptorCount = layout.descriptorCount;
	update.descriptorType = layout.descriptorType;
	update.imageInfo = { imageInfo };

	addUpdate(update, setIndex);
}

void DescriptorSet::addUpdate(VkAccelerationStructureKHR as, uint32_t setIndex, uint32_t binding)
{
	VkDescriptorSetLayoutBinding layout = bindings[binding];
	DescriptorSetUpdate update = {};
	update.dstBinding = layout.binding;
	update.descriptorCount = layout.descriptorCount;
	update.descriptorType = layout.descriptorType;
	update.accelerationStructure = as;
	addUpdate(update, setIndex);
}




void DescriptorSet::processUpdates(VkDevice device, uint32_t setIndex)
{
	//std::map<uint32_t, DescriptorSetUpdate>::iterator it;
	std::vector<DescriptorSetUpdate> updateVector;
	MapToVec(updates[setIndex], updateVector);
	std::vector< VkWriteDescriptorSet> descriptorSetWrites;
	std::vector< VkWriteDescriptorSetAccelerationStructureKHR> descriptorSetAsWrites;
	if (updates[setIndex].empty())
	{
		return;
	}
	for (uint32_t i = 0; i< updateVector.size(); i++)
	{
		VkWriteDescriptorSet descriptorWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptorWrite.dstSet = set[setIndex];
		descriptorWrite.dstBinding = updateVector[i].dstBinding;
		descriptorWrite.descriptorCount = updateVector[i].descriptorCount;
		descriptorWrite.descriptorType = updateVector[i].descriptorType;
		descriptorWrite.pImageInfo = updateVector[i].imageInfo.data();
		descriptorWrite.pBufferInfo = updateVector[i].bufferInfo.data();
		if (updateVector[i].descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
		{
			VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
			accelerationStructureWrite.accelerationStructureCount = 1;
			accelerationStructureWrite.pAccelerationStructures = &updateVector[i].accelerationStructure;
			descriptorSetAsWrites.push_back(accelerationStructureWrite);
			descriptorWrite.pNext = &descriptorSetAsWrites.back();
		}
		descriptorSetWrites.push_back(descriptorWrite);
	}
	//std::cout << "accelerationStructureWrite" << descriptorSetWrites
	vkUpdateDescriptorSets(device, descriptorSetWrites.size(), descriptorSetWrites.data(), 0, nullptr);
	updates[setIndex].clear();
}

void DescriptorSet::createLayout(VkDevice device)
{
	std::vector<VkDescriptorSetLayoutBinding> bindingsVector;
	MapToVec(bindings, bindingsVector);

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	descriptorSetLayoutCreateInfo.bindingCount = bindingsVector.size();
	descriptorSetLayoutCreateInfo.pBindings = bindingsVector.data();

	VkResult result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &layout);
}

void DescriptorSet::createSets(VkDevice device, uint32_t setCount, VkDescriptorPool pool)
{
	std::vector<VkDescriptorSetLayout> layouts(setCount, layout);
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	descriptorSetAllocateInfo.descriptorPool = pool;
	descriptorSetAllocateInfo.descriptorSetCount = MAX_FRAME_DRAWS;
	descriptorSetAllocateInfo.pSetLayouts = layouts.data();

	set.resize(MAX_FRAME_DRAWS);

	VkResult result = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, set.data());
	ASSERT_VULKAN(result);
}


std::vector<VkDescriptorPoolSize> DescriptorSet::getPoolSizes(uint32_t setCount)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	std::map<uint32_t, VkDescriptorSetLayoutBinding>::iterator it;
	for (it = bindings.begin(); it != bindings.end(); it++)
	{
		poolSizes.push_back({ it->second.descriptorType, setCount }); // Civ
	}
	return poolSizes;
}
