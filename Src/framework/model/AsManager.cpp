#include "AsManager.h"
#include "DrawGeom.h"
#include "RtUtility.h"


AsManager::AsManager(ResourceManager* rm)
{
	this->rm = rm;
	this->context = rm->context;
}

AsManager::~AsManager()
{
}

void AsManager::uploadInstanceBuffers(std::vector<DrawInst*> inst, bool recreate)
{
	VK_LOAD(vkGetAccelerationStructureDeviceAddressKHR);

	if (recreate)
	{
		rm->addToDeleteQueue(&instanceBufferKHR, 1);
		rm->addToDeleteQueue(&instanceDescriptorBuffer, 1);
	}

	// Create instancesKHR

	uint32_t size = sizeof(VkAccelerationStructureInstanceKHR);

	std::vector<VkAccelerationStructureInstanceKHR> instancesKHR;
	std::vector<uint32_t> sbtInstanceOffsets = vk_rt::getInstanceShaderBindingTableRecordOffsets(inst, RAY_TYPES);
	for (uint32_t i = 0; i < inst.size(); i++)
	{
		VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
		addressInfo.accelerationStructure = inst[i]->geometry->blas.as;
		VkDeviceAddress blasAddress = pvkGetAccelerationStructureDeviceAddressKHR(context->device, &addressInfo);

		// Civ
		inst[i]->customIntersectionIndex = i;

		VkAccelerationStructureInstanceKHR instKHR{};
		instKHR.transform = vk_rt::getTransformMatrix(inst[i]->shaderData.modelMat);
		instKHR.instanceCustomIndex = i;
		instKHR.mask = 0xFF;
		instKHR.instanceShaderBindingTableRecordOffset = sbtInstanceOffsets[i];
		instKHR.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR;
		instKHR.accelerationStructureReference = blasAddress;

		instancesKHR.push_back(instKHR);
	}

	//std::cout << "instance count: " << inst.size() << std::endl;
	VkDeviceSize instanceDescsSizeInBytes = inst.size() * sizeof(VkAccelerationStructureInstanceKHR);

	// Create instance buffer
	//rm->createAndUploadDeviceBuffer(cmdBuf, instanceBufferKHR, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, instancesKHR);
	rm->immediateCreateAndUploadDeviceBuffer(instanceBufferKHR, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, instancesKHR);
	// Create instance descriptor buffer
	std::vector<InstanceDescriptor> instanceDescriptors;
	for (uint32_t i = 0; i < inst.size(); i++)
	{
		InstanceDescriptor desc{};
		desc.modelMat = inst[i]->shaderData.modelMat;
		desc.vertices = getBufferAddress(context->device, inst[i]->geometry->vertexBuffer.buffer);
		desc.indices = getBufferAddress(context->device, inst[i]->geometry->indexBuffer.buffer);
		desc.materials = getBufferAddress(context->device, inst[i]->geometry->materialBuffer.buffer);
		instanceDescriptors.push_back(desc);
	}
	//rm->createAndUploadDeviceBuffer(cmdBuf, instanceDescriptorBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, instanceDescriptors);
	rm->immediateCreateAndUploadDeviceBuffer(instanceDescriptorBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, instanceDescriptors);
	instanceCount = inst.size();
	// Add buffer to delete queue
	//rm->addToDeleteQueue(&instanceDescriptorBuffer, 1);
}
void AsManager::buildTLAS()
{
	VK_LOAD(vkGetAccelerationStructureBuildSizesKHR);
	VK_LOAD(vkCmdBuildAccelerationStructuresKHR);
	VK_LOAD(vkCreateAccelerationStructureKHR);

	VkCommandBuffer cmdBuf = rm->startSingleTimeCmdBuffer();
	// Get device address
	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferInfo.buffer = instanceBufferKHR.buffer;
	VkDeviceAddress instanceAddress = vkGetBufferDeviceAddress(context->device, &bufferInfo);

	// Add barrier
	VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		0, 1, &barrier, 0, nullptr, 0, nullptr);

	// Create and build tlas
	VkAccelerationStructureGeometryInstancesDataKHR instancesVk{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
	instancesVk.arrayOfPointers = VK_FALSE;
	instancesVk.data.deviceAddress = instanceAddress;

	VkAccelerationStructureGeometryKHR topASGeometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	topASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	topASGeometry.geometry.instances = instancesVk;

	// Find sizes
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	buildInfo.flags = TLAS_BUILD_FLAGS;
	buildInfo.geometryCount = 1;
	buildInfo.pGeometries = &topASGeometry;
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

	uint32_t                                 count = (uint32_t)instanceCount;
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
	pvkGetAccelerationStructureBuildSizesKHR(context->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &count, &sizeInfo);

	VkAccelerationStructureCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	createInfo.size = sizeInfo.accelerationStructureSize;

	// Create tlas buffer
	Buffer asBuffer = rm->createTemporaryBuffer(
		createInfo.size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);
	createInfo.buffer = asBuffer.buffer;

	// Create tlas
	pvkCreateAccelerationStructureKHR(context->device, &createInfo, nullptr, &tlas);

	// Add as to delete queue
	rm->addToDeleteQueue(&tlas, 1);

	// Create scratch buffer for build
	Buffer scratchBuffer = rm->createTemporaryBuffer(
		createInfo.size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	// Get scratch buffer address
	bufferInfo.buffer = scratchBuffer.buffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(context->device, &bufferInfo);

	// Update build information
	buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	buildInfo.dstAccelerationStructure = tlas;
	buildInfo.scratchData.deviceAddress = scratchAddress;

	// Build Offsets info
	VkAccelerationStructureBuildRangeInfoKHR        buildOffsetInfo{ static_cast<uint32_t>(instanceCount), 0, 0, 0 };
	const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = &buildOffsetInfo;

	// Build the TLAS
	pvkCmdBuildAccelerationStructuresKHR(cmdBuf, 1, &buildInfo, &pBuildOffsetInfo);

	// Finaly Submit buffer
	SubmitSynchronizationInfo syncInfo{};
	syncInfo.pSignalSemaphore = &context->asFinished[context->currentSwapchainIndex];
	syncInfo.signalSemaphoreCount = 1;

	rm->cmdBufEndAndSubmitSynchronized(&cmdBuf, 1, syncInfo, true);
}
