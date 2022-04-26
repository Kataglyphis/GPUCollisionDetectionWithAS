#include "DescriptorSetManager.h"
#include "Utilities.h"

#define ADD_DESCRIPTOR_UPDATES(R)\
for (resource_binding descInfo : descInfos)\
{\
	if (allSets)\
	{\
		for (uint32_t i = 0; i < MAX_FRAME_DRAWS; i++)\
		{\
			sets[descInfo.set_index]->addUpdate((R), i, descInfo.binding);\
		}\
	}\
	else\
	{\
		sets[descInfo.set_index]->addUpdate((R), context->currentSwapchainIndex, descInfo.binding);\
	}\
}\



DescriptorSetManager::DescriptorSetManager(Context* context)
{
	this->context = context;
	this->descriptorSetGeometryPass = new DescriptorSet(context->device, layout_definition_deferred_geometry_pass);
    this->sets.push_back(descriptorSetGeometryPass);

	this->descriptorSetShadingPass = new DescriptorSet(context->device, layout_definition_deferred_shading_pass);
    this->sets.push_back(descriptorSetShadingPass);

	this->descriptorSetParticlePass = new DescriptorSet(context->device, layout_definition_deferred_particel_pass);
    this->sets.push_back(descriptorSetParticlePass);

	this->descriptorSetRtPhysicsIntegration = new DescriptorSet(context->device, layout_definition_rt_intergration_pass);
    this->sets.push_back(descriptorSetRtPhysicsIntegration);

	this->descriptorSetRtPhysicsCollision = new DescriptorSet(context->device, layout_definition_rt_collision_pass);
    this->sets.push_back(descriptorSetRtPhysicsCollision);

	this->descriptorSetPathTracing = new DescriptorSet(context->device, layout_definition_path_tracing);
	this->sets.push_back(descriptorSetPathTracing);

	this->descriptorSetClassicPhysics = new DescriptorSet(context->device, layout_definition_classic_physics);
	this->sets.push_back(descriptorSetClassicPhysics);

	this->descriptorSetDepthPeelingPass = new DescriptorSet(context->device, layout_definition_depth_peeling);
	this->sets.push_back(descriptorSetDepthPeelingPass);



	initDescriptorSets();
}

void DescriptorSetManager::initDescriptorSets()
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    auto addToPool = [&](DescriptorSet* set)
    {
        std::vector<VkDescriptorPoolSize> setPoolSizes = set->getPoolSizes(MAX_FRAME_DRAWS);
        poolSizes.insert(poolSizes.end(), setPoolSizes.begin(), setPoolSizes.end());
    };

    for (uint32_t i = 0; i < sets.size(); i++)
    {
        addToPool(sets[i]);
    }

    createDescriptorPool(context->device, poolSizes, sets.size() * MAX_FRAME_DRAWS, &descriptorPool);

    for (uint32_t i = 0; i < sets.size(); i++)
    {
        sets[i]->createSets(context->device, MAX_FRAME_DRAWS, descriptorPool);
    }
}

void DescriptorSetManager::addDescriptorUpdate(Buffer buf, std::vector<resource_binding> descInfos, bool allSets)
{
	ADD_DESCRIPTOR_UPDATES(buf)
}

void DescriptorSetManager::addDescriptorUpdate(std::vector<Image> images, std::vector<resource_binding> descInfos, bool allSets)
{
	ADD_DESCRIPTOR_UPDATES(images)
}

void DescriptorSetManager::addDescriptorUpdate(VkSampler sampler, std::vector<resource_binding> descInfos, bool allSets)
{
	ADD_DESCRIPTOR_UPDATES(sampler)
}

void DescriptorSetManager::addDescriptorUpdate(VkAccelerationStructureKHR as, std::vector<resource_binding> descInfos, bool allSets)
{
	ADD_DESCRIPTOR_UPDATES(as)
}


DescriptorSetManager::~DescriptorSetManager()
{
}