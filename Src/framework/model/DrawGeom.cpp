#include "DrawGeom.h"



DrawGeom::DrawGeom(ResourceManager* rm, Model* model, uint32_t maxInstances)
{
	this->context = rm->context;
	this->model = model;
	this->numVertices = model->vertices.size();
	this->numIndices = model->indices.size();
	this->vertexBuffer = {};
	this->indexBuffer = {};

	this->blas = {};
	std::vector<DrawInst> instances = {};
	bool hasBlas = false;

	// Add surfaces
	std::vector<Material> materials;
	for (uint32_t i = 0; i < model->surfaces.size(); i++)
	{
		Surface surface = model->surfaces[i];
		DrawSurf::Opacity opacity = (surface.mat.tex_idx_coverage == UINT32_MAX) ? DrawSurf::Opacity::OPAQUE : DrawSurf::Opacity::CUT_OUT;
		surfs.push_back(DrawSurf(this, surface.startIndex, surface.numIndices, 0, opacity));

		Material mat = surface.mat;
		mat.indexOffset = surface.startIndex;
		materials.push_back(mat);
	}

	rm->immediateCreateAndUploadDeviceBuffer(this->vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, model->vertices);
	rm->immediateCreateAndUploadDeviceBuffer(this->indexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, model->indices);
	rm->immediateCreateAndUploadDeviceBuffer(this->materialBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, materials);


	VkBufferDeviceAddressInfo addr{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	addr.buffer = materialBuffer.buffer;
	this->materialBufferAddress = vkGetBufferDeviceAddress(context->device, &addr);

	//std::vector<Vertex> vertexBufferData(this->numVertices);
	//std::vector<Index> indexBufferData(this->numIndices);
	//
	//rm->immediateDownloadDeviceBuffer(this->vertexBuffer, vertexBufferData);
	//rm->immediateDownloadDeviceBuffer(this->indexBuffer, indexBufferData);

	setDebugMarker(rm->context->device, VK_OBJECT_TYPE_BUFFER, (uint64_t)this->vertexBuffer.buffer, "Vertex Buffer");
	setDebugMarker(rm->context->device, VK_OBJECT_TYPE_BUFFER, (uint64_t)this->indexBuffer.buffer, "Index Buffer");

	createBLAS();
	createInstanceBuffer(maxInstances);
	createIndirectDrawBuffer(rm);
}

DrawGeom* DrawGeom::getDefaultCube(ResourceManager* rm)
{
	return new DrawGeom(rm, &Model::getDefaultCube(), MAX_INSTANCES);
}

void DrawGeom::getDefaultCubeVoxelGrid(ResourceManager* rm)
{
	if (hasVoxelGrid)
	{
		rm->addToDeleteQueue(&voxelGridBuffer, 1);
		//rm->addToDeleteQueue(&voxelGridIndexBuffer, 1);
	}
	//VkCommandBuffer cmdBuf = rm->startSingleTimeCmdBuffer();
	// Pass in
	float extent = 1.0;
	float voxelSize = 0.2;
	uint32_t rowLenght = (extent / voxelSize);
	uint32_t planeSize = rowLenght*rowLenght;
	numVoxels = rowLenght*planeSize;
	std::vector<glm::vec4> voxelGrid(numVoxels);
	//glm::vec4* data = (glm::vec4*)calloc(numVoxels, sizeof(glm::vec4));
	glm::vec4 offset = glm::vec4(-extent / 2.0 + voxelSize*0.5, -extent / 2.0 + voxelSize * 0.5, -extent / 2.0 + voxelSize * 0.5, 0.0);
	for (uint32_t x = 0; x < rowLenght; x++)
	{
		for (uint32_t y = 0; y < rowLenght; y++)
		{
			for (uint32_t z = 0; z < rowLenght; z++)
			{
				uint32_t idx = x * planeSize + y * rowLenght + z;
				glm::vec4 pos = offset + glm::vec4(
					(float)x / (float)rowLenght,
					(float)y / (float)rowLenght,
					(float)z / (float)rowLenght,
					0.0);
				voxelGrid[idx] = pos;
			}
		}
	}
	//rm->createAndUploadDeviceBuffer(cmdBuf, voxelGridBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, voxelGrid);
	rm->immediateCreateAndUploadDeviceBuffer(voxelGridBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, voxelGrid);
	VkBufferDeviceAddressInfo addr{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	addr.buffer = voxelGridBuffer.buffer;
	voxelGridBufferAddress = vkGetBufferDeviceAddress(context->device, &addr);
	//void* data = (glm::vec4*)calloc(numVoxels*instances.size(), sizeof(uint32_t));
	//rm->createAndUploadDataToDeviceBuffer(cmdBuf, voxelGridIndexBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, data,
	//	(uint32_t)(numVoxels * instances.size()* sizeof(uint32_t)));
	hasVoxelGrid = true;
}

void DrawGeom::addInstance(ResourceManager* rm, InstanceShaderData shaderData)
{
	uint32_t index = instances.size();
	instances.push_back(DrawInst(shaderData, this));
	shaderData.oldModelMat = shaderData.modelMat;
	rm->immediatWriteToDeviceBuffer(instanceBuffer, &shaderData, sizeof(InstanceShaderData), index * sizeof(InstanceShaderData));
	createIndirectDrawBuffer(rm);
}

void DrawGeom::addInstances(ResourceManager* rm, std::vector<DrawInst> newInstances)
{
	uint32_t index = instances.size();
	instances.insert(instances.end(), newInstances.begin(), newInstances.end());
	std::vector<InstanceShaderData> shaderData(newInstances.size());
	for (uint32_t i = 0; i < newInstances.size(); i++)
	{
		shaderData[i] = newInstances[i].shaderData;
		shaderData[i].oldModelMat = newInstances[i].shaderData.modelMat;
	}
	rm->immediatWriteToDeviceBuffer(instanceBuffer, shaderData.data(), shaderData.size()*sizeof(InstanceShaderData), index * sizeof(InstanceShaderData));
	createIndirectDrawBuffer(rm);
}


void DrawGeom::createInstanceBuffer(uint32_t maxInstances)
{
	createBufferWithAllocator(context->device, context->allocator,
		maxInstances * sizeof(InstanceShaderData),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		instanceBuffer.buffer,
		instanceBuffer.allocation);
	hasInstanceBuffer = true;

	VkBufferDeviceAddressInfo addr{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	addr.buffer = instanceBuffer.buffer;
	instanceBufferAddress = vkGetBufferDeviceAddress(context->device, &addr);
}

void DrawGeom::createIndirectDrawBuffer(ResourceManager* rm)
{
	if (hasIndirectDrawBuffer)
	{
		rm->addToDeleteQueue(&indirectDrawBuffer, 1);
	}
	createBufferWithAllocator(context->device, context->allocator,
		sizeof(VkDrawIndexedIndirectCommand) * surfs.size(), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
		| VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		, VMA_MEMORY_USAGE_GPU_TO_CPU,
		indirectDrawBuffer.buffer, indirectDrawBuffer.allocation);
	std::vector<VkDrawIndexedIndirectCommand> drawIndirectCmds;
	for (uint32_t i = 0; i < surfs.size(); i++)
	{
		VkDrawIndexedIndirectCommand drawIndirectCmd{};
		drawIndirectCmd.indexCount = surfs[i].numIndices;
		drawIndirectCmd.instanceCount = instances.size();
		drawIndirectCmd.firstIndex = surfs[i].firstIndex;
		drawIndirectCmd.vertexOffset = surfs[i].vertexBufferOffset;
		drawIndirectCmd.firstInstance = 0;
		drawIndirectCmds.push_back(drawIndirectCmd);
	}
	writeVectorToBuffer(context->device, context->allocator, indirectDrawBuffer, drawIndirectCmds);
	hasIndirectDrawBuffer = true;
}

void DrawGeom::updateInstances()
{
	std::vector<InstanceShaderData> instanceShaderData(instances.size());
	readVectorFromBuffer(context->device, context->allocator, instanceBuffer, instanceShaderData);
	for (uint32_t i = 0; i < instances.size(); i++)
	{
		instances[i].shaderData = instanceShaderData[i];
	}
}

DrawGeom::DrawGeom()
{
}


// Returns the old Blas
void DrawGeom::recordUpdateBLAS(VkCommandBuffer& cmdBuf, Buffer& scratchBuffer)
{
	VK_LOAD(vkCmdBuildAccelerationStructuresKHR);

	std::vector<VkAccelerationStructureGeometryKHR>					geomKHR			= getGeometryKHR();
	std::vector<VkAccelerationStructureBuildRangeInfoKHR>			buildRangeKHR	= getBuildRangeInfo();

	std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> pBuildRangeKHR(buildRangeKHR.size());
	for (size_t i = 0; i < buildRangeKHR.size(); i++)
	{
		pBuildRangeKHR[i] = &buildRangeKHR[i];
	}

	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferInfo.buffer = scratchBuffer.buffer;

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	buildInfo.flags = BLAS_BUILD_FLAGS;
	buildInfo.geometryCount = surfs.size();
	buildInfo.pGeometries = geomKHR.data();
	buildInfo.mode = (hasBlas)? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildInfo.srcAccelerationStructure = (hasBlas)? blas.as : VK_NULL_HANDLE;
	buildInfo.scratchData.deviceAddress = vkGetBufferDeviceAddress(context->device, &bufferInfo);
	buildInfo.dstAccelerationStructure = blas.as;

	// Building the AS
	pvkCmdBuildAccelerationStructuresKHR(cmdBuf, 1, &buildInfo, pBuildRangeKHR.data());

	VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
	hasBlas = true;
}

// Returns the scratch size
void DrawGeom::createBLAS()
{
	VK_LOAD(vkCreateAccelerationStructureKHR);
	VK_LOAD(vkGetAccelerationStructureBuildSizesKHR);

	std::vector<VkAccelerationStructureGeometryKHR>			geomKHR			= getGeometryKHR();
	std::vector<VkAccelerationStructureBuildRangeInfoKHR>	buildRangeKHR	= getBuildRangeInfo();

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	buildInfo.flags = BLAS_BUILD_FLAGS;
	buildInfo.geometryCount = geomKHR.size();
	buildInfo.pGeometries = geomKHR.data();
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	
	std::vector<uint32_t> maxPrimCount(buildRangeKHR.size());

	for (uint32_t i = 0; i < buildRangeKHR.size(); i++)
	{
		maxPrimCount[i] = buildRangeKHR[i].primitiveCount;
	}

	// Get sizes
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
	pvkGetAccelerationStructureBuildSizesKHR(context->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildInfo, maxPrimCount.data(), &sizeInfo);

	// Create acceleration structure object. Not yet bound to memory.
	VkAccelerationStructureCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	createInfo.size = sizeInfo.accelerationStructureSize;
	// As buffer
	createBufferDedicated(context->device, context->physDevice,
		createInfo.size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		blas.buffer);
	createInfo.buffer = blas.buffer.buffer;
	// Create the acceleration structure
	VkResult result = pvkCreateAccelerationStructureKHR(context->device, &createInfo, nullptr, &blas.as);
	ASSERT_VULKAN(result);

	blas.buildScratchSize = sizeInfo.buildScratchSize;
}

// Uses our Vertex definition
std::vector<VkAccelerationStructureGeometryKHR> DrawGeom::getGeometryKHR()
{
	VkDeviceAddress vertexAddress = getBufferAddress(context->device, vertexBuffer.buffer);
	VkDeviceAddress indexAddress = getBufferAddress(context->device, indexBuffer.buffer);

	std::vector<VkAccelerationStructureGeometryKHR> asGeom;
	for (uint32_t i = 0; i < surfs.size(); i++)
	{
		VkAccelerationStructureGeometryTrianglesDataKHR triangles = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
		triangles.vertexFormat = Vertex::getAttributeDescriptions()[0].format;
		triangles.vertexData.deviceAddress = vertexAddress;
		triangles.vertexStride = sizeof(Vertex);
		triangles.maxVertex = std::min(numVertices, surfs[i].numIndices);
		triangles.indexType = VK_INDEX_TYPE_UINT32;
		triangles.indexData.deviceAddress = indexAddress;
		triangles.transformData = {};

		VkAccelerationStructureGeometryKHR surfAsGeom = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		surfAsGeom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		surfAsGeom.geometry.triangles = triangles;
		surfAsGeom.flags = (surfs[i].opacity == DrawSurf::Opacity::OPAQUE)? VK_GEOMETRY_OPAQUE_BIT_KHR : 0;//Civ

		asGeom.push_back(surfAsGeom);
	}
	return asGeom;
}

std::vector <VkAccelerationStructureBuildRangeInfoKHR> DrawGeom::getBuildRangeInfo()
{
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> offsets;
	for (uint32_t i = 0; i < surfs.size(); i++)
	{
		
		VkAccelerationStructureBuildRangeInfoKHR surfOffset{};
		surfOffset.primitiveCount = surfs[i].numIndices / 3;
		surfOffset.primitiveOffset = surfs[i].firstIndex * sizeof(Index); //civ
		surfOffset.firstVertex = 0;
		surfOffset.transformOffset = 0;

		offsets.push_back(surfOffset);
	}
	return offsets;
}



DrawGeom::~DrawGeom()
{
}

