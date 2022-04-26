#include "Scene.h"
#include "PipelineToolKit.h"
#include <glm/gtx/quaternion.hpp>


#define PARTICLE_TEXTURE_SIZE_X 2048
#define PARTICLE_TEXTURE_SIZE_Y 2048

void Scene::updatePerFrameConstants()
{
	PerFrameConstants pfc = {};
	pfc.view = view.viewMatrix;
	pfc.projection = view.projectionMatrix;
	pfc.model = glm::mat4(1.0);//geom[0]->instances[0].modelMat;


	pfc.inverseView			= glm::inverse(pfc.view);
	pfc.inverseProjection	= glm::inverse(pfc.projection);
	pfc.directionalLight	= glm::vec4(0.0, -2.0, -1.0, 1.0); // For now (a = intensity)
	pfc.camPos				= glm::vec4(view.camPos, 1.0);
	pfc.height				= context->height;
	pfc.width				= context->width;
	pfc.cnt					= context->totalFrameIndex;
	pfc.particleModel		= context->particleModel;

	//pfc.projection = glm::ortho(-1.f,
	//							1.f,
	//							-1.f,
	//							1.f,
	//							0.1f, 5.f);

	//pfc.inverseProjection = glm::inverse(pfc.projection);

	//glm::perspective(glm::radians(60.0f), context->width / (float)context->height, 0.01f, 1000.0f); 
	//glm::vec3 position = glm::vec3(0,0,2);
	////retrieve the right vector with some world_up
	//glm::vec3 front = { 0.0f, 0.0f, -1.0f };
	//glm::vec3 world_up = { 0.0f, 1.0f, 0.0f };
	//glm::vec3 right = glm::normalize(glm::cross(front, world_up));

	//// but this means the up vector must again be calculated with right vector calculated!!!
	//glm::vec3 up = glm::normalize(glm::cross(right, front));
	//pfc.view = glm::lookAt(position, position + front, up);
	//pfc.inverseView = glm::inverse(pfc.view);

	std::vector<PerFrameConstants> pfcVector = { pfc };
	rm->createAndUploadDeviceBuffer(uploadCmdBuf, perFrameConstantsBuf, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, pfcVector);
	rm->addToDeleteQueue(&perFrameConstantsBuf, 1);
};


Scene::Scene(ResourceManager* rm, DescriptorSetManager* descriptorSetManager)
{
	this->rm = rm;
	this->context = rm->context;
	this->descSetManager = descriptorSetManager;
	this->texManager = new MaterialTextureManager(rm);
	this->particleTexManager = new MaterialTextureManager(rm);
	this->matManager = new MaterialManager(rm, this->texManager);
	this->objLoader = new ObjLoader(this->matManager);
	context->asManager = new AsManager(rm);
}


void Scene::createInstanceGrid(DrawGeom* geom, float scaleFactor, float distance)
{
	//glm::vec4 gridOffset = glm::vec4(10, 0.0, 70, 1.0);
	glm::vec4 gridOffset = g_grid_offset.val.getVec4();//glm::vec4(0.0, 0.0, 0.0, 0.0);
	InstanceShaderData instData{};
	instData.rotation = glm::vec4(-1.0, 0.0, 0.0, 0.0);
	instData.scale = glm::vec4(scaleFactor, scaleFactor, scaleFactor, 1.0);
	auto random = [&]() {return (1.0 - 2.0 * (((float)std::rand()) / (float)RAND_MAX)); };
	std::vector<DrawInst> instances;
	uint32_t gridSize_x = g_grid_size_x.val.v_uint;
	uint32_t gridSize_y = g_grid_size_y.val.v_uint;
	uint32_t gridSize_z = g_grid_size_z.val.v_uint;
	instances.reserve(gridSize_x * gridSize_y * gridSize_z);
	for (uint32_t x = 0; x < gridSize_x; x++)
	{
		for (uint32_t y = 0; y < gridSize_y; y++)
		{
			//uint32_t z = 0;
			for (uint32_t z = 0; z < gridSize_z; z++)
			{
				instData.velocity = glm::vec4(random() * 0.001, random() * 0.001, random()* 0.001, 1.0);
				instData.angularVelocity = glm::vec4(random() * 0.02, random() * 0.02, random() * 0.02, 1.0);
				//instData.position			= glm::vec4(distance * (x-gridSize/2), -20.0, distance * (y-gridSize/2),1.0);
				instData.position = gridOffset + glm::vec4(-distance * x + gridSize_x /2.0, -distance * z -20.0, -distance * y + gridSize_y / 2.0, 1.0);

				glm::quat	quat = glm::quat(instData.rotation.w, instData.rotation.x, instData.rotation.y, instData.rotation.z);
				instData.modelMat = glm::toMat4(quat);
				instData.modelMat = glm::translate(instData.modelMat, glm::vec3(instData.position));
				instData.modelMat = glm::scale(instData.modelMat, glm::vec3(instData.scale));
				instances.push_back(DrawInst(instData, geom));
				//geom->addInstance(rm, instData);
			}
		}
	}
	geom->addInstances(rm, instances);
#ifndef USE_DEPTH_PEELING
	geom->getDefaultCubeVoxelGrid(rm);
#endif // !USE_DEPTH_PEELING

}

void Scene::resetDynamicGeometry()
{
	geom[1]->instances.clear();
	createInstanceGrid(geom[1], cube.scale, 4.0);
	updateInstances();
}

void Scene::load(SceneLoadInfo loadInfo)
{
	
	// Load Scene
	Model* sceneModel = objLoader->load(loadInfo.objPath, loadInfo.mtlBaseDir, loadInfo.texturePath);
	DrawGeom* drawGeom = new DrawGeom(rm, sceneModel, 1);
	float scaleFactor = loadInfo.scale;

	//glm::mat4 matrix = glm::translate(glm::mat4(1.0), glm::vec3(0,0.5,0));
	//matrix = glm::scale(matrix, glm::vec3(100, 100, 100));
	
	glm::mat4 matrix = glm::scale(glm::mat4(1.0), glm::vec3(scaleFactor, scaleFactor, scaleFactor));
	drawGeom->addInstance(rm, { matrix });
	geom.push_back(drawGeom);

	// Load Cruiser
	//sceneModel = objLoader->load(cube.objPath, cube.mtlBaseDir, cube.texturePath);
#ifdef USE_DEPTH_PEELING
	sceneModel = objLoader->load(vikingRoom.objPath, vikingRoom.mtlBaseDir, vikingRoom.texturePath);
#else
	sceneModel = objLoader->load(cube.objPath, cube.mtlBaseDir, cube.texturePath);
#endif
	drawGeom = new DrawGeom(rm, sceneModel, MAX_INSTANCES);
	createInstanceGrid(drawGeom, cube.scale, 4.0);
	geom.push_back(drawGeom);
	//Model* sceneModel; DrawGeom* drawGeom;




	// Build blas
	SubmitSynchronizationInfo syncInfo{};
	rm->synchronisedUpdateBLAS(geom, syncInfo);
	VkResult result = vkDeviceWaitIdle(context->device);;
	ASSERT_VULKAN(result);
	
	// Upload materials
	matManager->uploadMaterials();

	// Get Image For Particle
	particleTexManager->addImage("../Resources/Textures/Particles/RenderTextures/red_spark.png", TEXTURE_FORMAT_DIFFUSE_ALBEDO);
	particleTexManager->addImage("../Resources/Textures/Particles/RenderTextures/csm_coronavirus.png", TEXTURE_FORMAT_DIFFUSE_ALBEDO);

	// Upload textures
	particleTexManager->uploadImages();
	texManager->uploadImages();

	materialBuffer = matManager->getMaterialBuffer();
	textures = texManager->getImages();
	particleTextures = particleTexManager->getImages();

	// Create sampler
	vk_init::createSampler(context->device, loadInfo.textureSize, loadInfo.textureSize, &sampler);
	vk_init::createSampler(context->device, PARTICLE_TEXTURE_SIZE_X, PARTICLE_TEXTURE_SIZE_Y, &particleSampler);

	// Update decriptor updates
	descSetManager->addDescriptorUpdate(materialBuffer, descSetManager->resourceBindings.material, true);
	descSetManager->addDescriptorUpdate(textures, descSetManager->resourceBindings.textures, true);
	descSetManager->addDescriptorUpdate(sampler, descSetManager->resourceBindings.sampler, true);
	descSetManager->addDescriptorUpdate(particleSampler, descSetManager->resourceBindings.particleSampler, true);
	descSetManager->addDescriptorUpdate(particleTextures, descSetManager->resourceBindings.particleTextures, true);


	view.projectionMatrix = glm::perspective(glm::radians(60.0f), context->width / (float)context->height, 0.01f, 10.0f);
	updateInstances();

#ifdef USE_DEPTH_PEELING


	this->depthPeeling = new DepthPeeling(rm, descSetManager);
	
	DrawParams params{};
	params.geom = geom;
	params.pVertexBufferSceneParticles = &depthPeeling->get_particle_representation_buffer()->buffer;
	Buffer* geomParticleBuffer = depthPeeling->get_particle_representation_buffer();
	params.vertexCountSceneParticles = context->width * context->height * MAX_FRAGMENTS * INSERTION_RATE;
	VkCommandBuffer cmdBuf = rm->startSingleTimeCmdBuffer();
	descSetManager->descriptorSetDepthPeelingPass->processUpdates(context->device, context->currentSwapchainIndex);
	depthPeeling->recordRenderCommands(cmdBuf, params);
	rm->cmdBufEndAndSubmit(&cmdBuf, 1, true, true);
	std::vector<Counter> counter(1);
	Buffer* counterBuffer = depthPeeling->get_counter_buffer();
	rm->immediateDownloadDeviceBuffer(*counterBuffer, counter);
	std::vector<Particle> output(counter[0].particle_count);
	rm->immediateDownloadDeviceBuffer(*geomParticleBuffer, output);
	
	std::vector<glm::vec4> voxelGrid(counter[0].particle_count);
	for (uint32_t i = 0; i < output.size(); i++)
	{
		voxelGrid[i] = output[i].position;
	}
	
	rm->immediateCreateAndUploadDeviceBuffer(geom[1]->voxelGridBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, voxelGrid);
	VkBufferDeviceAddressInfo addr{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	addr.buffer = geom[1]->voxelGridBuffer.buffer;
	geom[1]->voxelGridBufferAddress = vkGetBufferDeviceAddress(context->device, &addr);
	geom[1]->numVoxels = voxelGrid.size();
	geom[1]->hasVoxelGrid = true;
#endif // USE_DEPTH_PEELING



	//void* data = (glm::vec4*)calloc(numVoxels*instances.size(), sizeof(uint32_t));
	//rm->createAndUploadDataToDeviceBuffer(cmdBuf, voxelGridIndexBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, data,
	//	(uint32_t)(numVoxels * instances.size()* sizeof(uint32_t)));


}

Scene::~Scene()
{
}


void Scene::updateInstances()
{
	inst.clear();
	for (uint32_t i = 0; i < geom.size(); i++)
	{
		geom[i]->updateInstances();
		for (uint32_t j = 0; j < geom[i]->instances.size(); j++)
		{
			inst.push_back(&geom[i]->instances[j]);
		}
		//instanceList.insert(instanceList.end(), geom[i]->instances.begin(), geom[i]->instances.end());
	}
	context->asManager->uploadInstanceBuffers(inst, asInstancesInitialized);
	descSetManager->addDescriptorUpdate(context->asManager->instanceBufferKHR, descSetManager->resourceBindings.asInstances, true);
	asInstancesInitialized = true;
}

void Scene::update(Camera* camera)
{

	uploadCmdBuf = rm->startSingleTimeCmdBuffer();
	setDebugMarker(rm->context->device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)uploadCmdBuf, "Upload Cmd Buf");
	// Update camera;
	view.viewMatrix = camera->calculate_viewmatrix();
	view.camPos = camera->get_camera_position();
	updatePerFrameConstants();

	// Build tlas
	//updateInstances();
	context->asManager->buildTLAS();



	// Update descriptors
	descSetManager->addDescriptorUpdate(context->asManager->tlas, descSetManager->resourceBindings.as, false);
	descSetManager->addDescriptorUpdate(context->asManager->instanceDescriptorBuffer, descSetManager->resourceBindings.instanceDescriptors, false);
	descSetManager->addDescriptorUpdate(perFrameConstantsBuf, descSetManager->resourceBindings.perFrameConstants, false);



	vkEndCommandBuffer(uploadCmdBuf);

	SubmitSynchronizationInfo syncInfo{};
	syncInfo.signalSemaphoreCount = 1;
	syncInfo.pSignalSemaphore = &context->uploadFinishedSemaphores[context->currentSwapchainIndex];

	rm->cmdBufSubmitSynchronized(&uploadCmdBuf, 1, syncInfo, true);

}
