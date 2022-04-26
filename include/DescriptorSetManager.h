#pragma once
#include "DescriptorSet.h"
#include "Context.h"

struct resource_binding
{
	uint32_t set_index;
	uint32_t binding;
};

struct resource_descriptor_set_bindings
{
	std::vector<resource_binding>					perFrameConstants;
	std::vector<resource_binding>					sampler;
	std::vector<resource_binding>					textures;
	std::vector<resource_binding>					material;
	std::vector<resource_binding>					particleSampler;
	std::vector<resource_binding>					particleTextures;
	std::vector<resource_binding>					instanceDescriptors;
	std::vector<resource_binding>					as;
	std::vector<resource_binding>					offscreenImage;
	std::vector<resource_binding>					asInstances;
	std::vector<resource_binding>					depthPeelingConstants;
	std::vector<resource_binding>					depthPeelingCounter;
	std::vector<resource_binding>					depthPeelingLinkedList;
	std::vector<resource_binding>					depthPeelingHeadPointer;
	std::vector<resource_binding>					depthPeelingParticleBuffer;
	std::vector<resource_binding>					globalVoxelGrid;
	std::vector<resource_binding>					instanceData;

};


class DescriptorSetManager
{
public:
	DescriptorSetManager(Context* context);
	void initDescriptorSets();
	void addDescriptorUpdate(Buffer buf, std::vector<resource_binding> descInfos, bool allSets);
	void addDescriptorUpdate(std::vector<Image> images, std::vector<resource_binding> descInfos, bool allSets);
	void addDescriptorUpdate(VkSampler sampler, std::vector<resource_binding> descInfos, bool allSets);
	void addDescriptorUpdate(VkAccelerationStructureKHR as, std::vector<resource_binding> descInfos, bool allSets);
	~DescriptorSetManager();

	VkDescriptorPool descriptorPool;

	DescriptorSet* descriptorSetGeometryPass;
	DescriptorSet* descriptorSetShadingPass;
	DescriptorSet* descriptorSetParticlePass;
	DescriptorSet* descriptorSetRtPhysicsIntegration;
	DescriptorSet* descriptorSetRtPhysicsCollision;
	DescriptorSet* descriptorSetPathTracing;
	DescriptorSet* descriptorSetClassicPhysics;
	DescriptorSet* descriptorSetDepthPeelingPass;

	std::vector<DescriptorSet*> sets;

	const resource_descriptor_set_bindings resourceBindings
	{
		// Set index, layout index
		//perFrameConstants
		{{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6,0}},

		//sampler
		{ {0, 1}, {5, 3}},

		//textures
		{ {0, 2}, {5, 4}},

		// materials
		{ {0, 3}},

		//particle sampler
		{ { 2, 1} },

		// particle textures
		{ { 2, 2}, { 2, 3}},

		// instance descriptors
		{ { 1, 5}, {5, 1}},

		// as
		{ { 1, 6}, { 4, 2}, {5, 2}},

		// Offsceen image
		{ {5 , 5}, {1, 7} },

		// As instances
		{ {3 , 2} },

		// depth peeling uniforms
		{ {7 , 0} }, 

		// depth peeling counter
		{ {7 , 1} }, 

		// depth peeling linked list 
		{ {7 , 2} }, 

		// depth peeling head pointer
		{ {7 , 3} }, 

		// depth peeling particle buffer
		{ {7 , 4} },

		// global voxel grid
		{ {6 , 2} },

		// instance data
		{ {3 , 1}, {4 , 1}, {6 , 1} }

	};
private:
	Context* context;



};
