#include "ComputeParticles.h"
#include <AsManager.h>

ComputeParticles::ComputeParticles()
{
}

ComputeParticles::ComputeParticles(ResourceManager* rm, Camera* camera)
{
	// store a simple pointer to our general context
	this->context = rm->context;
	this->resource_manager = rm;

	this->camera = camera;

	vkGetPhysicalDeviceProperties(context->physDevice, &physicalDeviceProps);

	// save the limits for handling all special cases later on 
	context->compute_limits.maxComputeWorkGroupCount[0] = physicalDeviceProps.limits.maxComputeWorkGroupCount[0];
	context->compute_limits.maxComputeWorkGroupCount[1] = physicalDeviceProps.limits.maxComputeWorkGroupCount[1];
	context->compute_limits.maxComputeWorkGroupCount[2] = physicalDeviceProps.limits.maxComputeWorkGroupCount[2];

	context->compute_limits.maxComputeWorkGroupInvocations = physicalDeviceProps.limits.maxComputeWorkGroupInvocations;

	context->compute_limits.maxComputeWorkGroupSize[0] = physicalDeviceProps.limits.maxComputeWorkGroupSize[0];
	context->compute_limits.maxComputeWorkGroupSize[1] = physicalDeviceProps.limits.maxComputeWorkGroupSize[1];
	context->compute_limits.maxComputeWorkGroupSize[2] = physicalDeviceProps.limits.maxComputeWorkGroupSize[2];

	queryResults.resize(context->queryCount);

	compute_timing_tester = nullptr;

	create_synchronization_objects();
	create_descriptor_pool();
	create_uniform_buffers();
	create_vector_fields();
	create_particle_buffer();
	create_descriptor_set();
	create_compute_pipeline();
	//create_command_buffers();

}

void ComputeParticles::reloadShaders()
{
	vkDeviceWaitIdle(context->device);

	vkDestroyPipelineLayout(context->device, compute_pipeline_layout, nullptr);

	vkDestroyPipeline(context->device, compute_pipeline_integration, nullptr);
	vkDestroyPipeline(context->device, compute_pipeline_simulation, nullptr);

	// compile shader method from jonas
	compile_shaders();

	create_compute_pipeline();

}

void ComputeParticles::compute()
{
	update_descriptor_set(context->currentSwapchainIndex);
	create_command_buffers();
}

void ComputeParticles::clean_up()
{

	clean_up_vector_field();

	clean_up_particle_buffer();

	// --- COMPUTE STUFF
	vkDestroyPipelineLayout(context->device, compute_pipeline_layout, nullptr);

	for (int i = 0; i < MAX_FRAME_DRAWS; i++) {

		vkDestroyDescriptorSetLayout(context->device, compute_descriptor_set_layouts[i], nullptr);

	}

	vkDestroyPipeline(context->device, compute_pipeline_integration, nullptr);
	vkDestroyPipeline(context->device, compute_pipeline_simulation, nullptr);

}

ComputeParticles::~ComputeParticles()
{
	clean_up();
}

void ComputeParticles::create_synchronization_objects()
{

	context->renderComputeSync.resize(MAX_FRAME_DRAWS);

	for (int x = 0; x < MAX_FRAME_DRAWS; x++) {
		context->renderComputeSync[x] = VK_NULL_HANDLE;
	}

	VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	for (int x = 0; x < MAX_FRAME_DRAWS; x++) {

		VkResult result = vkCreateSemaphore(context->device, &semaphoreCreateInfo, NULL, &(context->renderComputeSync[x]));
		ASSERT_VULKAN(result);
		
	}

}

void ComputeParticles::create_descriptor_set()
{

	compute_descriptor_sets.resize(MAX_FRAME_DRAWS);
	compute_descriptor_set_layouts.resize(MAX_FRAME_DRAWS);

	VkDescriptorSetLayoutBinding set_layout_binding_particle_buffer{};
	set_layout_binding_particle_buffer.binding = C_PARTICLE_STORAGE_BUFFER_BINDING;
	set_layout_binding_particle_buffer.descriptorCount = 1;
	set_layout_binding_particle_buffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	set_layout_binding_particle_buffer.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutBinding set_layout_binding_uniform_buffer{};
	set_layout_binding_uniform_buffer.binding = C_UNIFORM_BUFFER_BINDING;
	set_layout_binding_uniform_buffer.descriptorCount = 1;
	set_layout_binding_uniform_buffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	set_layout_binding_uniform_buffer.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	std::vector<VkDescriptorSetLayoutBinding> layout_bindings_vector_fields{};
	layout_bindings_vector_fields.resize(NUM_VECTOR_FIELDS);
	
	for (int i = 0; i < NUM_VECTOR_FIELDS; i++) {

		layout_bindings_vector_fields[i].binding = C_VECTOR_FIELD_BINDING + i;
		layout_bindings_vector_fields[i].descriptorCount = 1;
		layout_bindings_vector_fields[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layout_bindings_vector_fields[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	}

	VkDescriptorSetLayoutBinding set_layout_binding_tlas{};
	set_layout_binding_tlas.binding = C_TLAS_BINDING;
	set_layout_binding_tlas.descriptorCount = 1;
	set_layout_binding_tlas.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	set_layout_binding_tlas.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutBinding set_layout_binding_object_description{};
	set_layout_binding_object_description.binding = C_OBJECT_DESCRIPTION_BINDING;
	set_layout_binding_object_description.descriptorCount = 1;
	set_layout_binding_object_description.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	set_layout_binding_object_description.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = {	set_layout_binding_particle_buffer ,
																		set_layout_binding_uniform_buffer,
																		set_layout_binding_tlas,
																		set_layout_binding_object_description };

	set_layout_bindings.insert(set_layout_bindings.end(),	layout_bindings_vector_fields.begin(), 
															layout_bindings_vector_fields.end());

	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
	descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
	descriptor_set_layout_create_info.pBindings = set_layout_bindings.data();

	for (int i = 0; i < MAX_FRAME_DRAWS; i++) {

		VkResult result = vkCreateDescriptorSetLayout(	context->device, &descriptor_set_layout_create_info, nullptr,
														&compute_descriptor_set_layouts[i]);

		if (result != VK_SUCCESS) {

			throw std::runtime_error("Failed to create a compute set layout!");

		}
	}

	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = compute_descriptor_pool;
	alloc_info.descriptorSetCount = static_cast<uint32_t>(compute_descriptor_sets.size());
	alloc_info.pSetLayouts = compute_descriptor_set_layouts.data();

	VkResult result = vkAllocateDescriptorSets(context->device, &alloc_info, compute_descriptor_sets.data());

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to allocate a compute descriptor set!");

	}

}

void ComputeParticles::update_descriptor_set(uint32_t frame_index)
{

	if (context->vectorFieldChanged) {
		vkDeviceWaitIdle(context->device);
		context->vectorFieldChanged = false;
		clean_up_vector_field();
		create_vector_fields();

	}

	if (context->numParticlesChanged) {
		vkDeviceWaitIdle(context->device);
		context->numParticlesChanged = false;
		clean_up_particle_buffer();
		create_particle_buffer();

	}

	// prepare for update descriptor sets 
	VkDescriptorBufferInfo particle_storage_buffer_info{};
	particle_storage_buffer_info.buffer = storage_buffer_computation.buffer;
	particle_storage_buffer_info.offset = 0;
	particle_storage_buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet partice_buffer_storage_buffer_write{};
	partice_buffer_storage_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	partice_buffer_storage_buffer_write.dstSet = compute_descriptor_sets[frame_index];
	partice_buffer_storage_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	partice_buffer_storage_buffer_write.dstBinding = C_PARTICLE_STORAGE_BUFFER_BINDING;
	partice_buffer_storage_buffer_write.pBufferInfo = &particle_storage_buffer_info;
	partice_buffer_storage_buffer_write.descriptorCount = 1;

	VkDescriptorBufferInfo uniform_buffer_info{};
	uniform_buffer_info.buffer = compute_uniform_buffer.buffer;
	uniform_buffer_info.offset = 0;
	uniform_buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet uniform_buffer_write{};
	uniform_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniform_buffer_write.dstSet = compute_descriptor_sets[frame_index];
	uniform_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_buffer_write.dstBinding = C_UNIFORM_BUFFER_BINDING;
	uniform_buffer_write.pBufferInfo = &uniform_buffer_info;
	uniform_buffer_write.descriptorCount = 1;

	std::vector<VkDescriptorImageInfo> vector_fields_image_info;
	vector_fields_image_info.resize(NUM_VECTOR_FIELDS);

	for (int i = 0; i < NUM_VECTOR_FIELDS; i++) {

		vector_fields_image_info[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vector_fields_image_info[i].imageView	= vector_field_images[i].view;
		vector_fields_image_info[i].sampler		= vector_field_samplers[i];

	}

	std::vector<VkWriteDescriptorSet> vector_fields_image_writes;
	vector_fields_image_writes.resize(NUM_VECTOR_FIELDS);

	for (int i = 0; i < NUM_VECTOR_FIELDS; i++) {

		vector_fields_image_writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vector_fields_image_writes[i].dstSet = compute_descriptor_sets[frame_index];
		vector_fields_image_writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		vector_fields_image_writes[i].dstBinding = C_VECTOR_FIELD_BINDING + i;
		vector_fields_image_writes[i].pImageInfo = &vector_fields_image_info[i];
		vector_fields_image_writes[i].descriptorCount = 1;
		
	}

	VkWriteDescriptorSetAccelerationStructureKHR descriptor_set_acceleration_structure{};
	descriptor_set_acceleration_structure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	descriptor_set_acceleration_structure.pNext = nullptr;
	descriptor_set_acceleration_structure.accelerationStructureCount = 1;
	descriptor_set_acceleration_structure.pAccelerationStructures = &(context->asManager->tlas);

	VkWriteDescriptorSet write_descriptor_set_acceleration_structure{};
	write_descriptor_set_acceleration_structure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set_acceleration_structure.pNext = &descriptor_set_acceleration_structure;
	write_descriptor_set_acceleration_structure.dstSet = compute_descriptor_sets[frame_index];
	write_descriptor_set_acceleration_structure.dstBinding = C_TLAS_BINDING;
	write_descriptor_set_acceleration_structure.dstArrayElement = 0;
	write_descriptor_set_acceleration_structure.descriptorCount = 1;
	write_descriptor_set_acceleration_structure.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	write_descriptor_set_acceleration_structure.pImageInfo = nullptr;
	write_descriptor_set_acceleration_structure.pBufferInfo = nullptr;
	write_descriptor_set_acceleration_structure.pTexelBufferView = nullptr;

	VkDescriptorBufferInfo object_description_buffer_info{};
	object_description_buffer_info.buffer = context->asManager->instanceDescriptorBuffer.buffer;
	object_description_buffer_info.offset = 0;
	object_description_buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet object_description_buffer_write{};
	object_description_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	object_description_buffer_write.dstSet = compute_descriptor_sets[frame_index];
	object_description_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	object_description_buffer_write.dstBinding = C_OBJECT_DESCRIPTION_BINDING;
	object_description_buffer_write.pBufferInfo = &object_description_buffer_info;
	object_description_buffer_write.descriptorCount = 1;

	std::vector<VkWriteDescriptorSet> write_descriptor_sets = { partice_buffer_storage_buffer_write ,
																uniform_buffer_write,
																write_descriptor_set_acceleration_structure,
																object_description_buffer_write };

	write_descriptor_sets.insert(write_descriptor_sets.end(),	vector_fields_image_writes.begin(), 
																vector_fields_image_writes.end());

	vkUpdateDescriptorSets(context->device,	static_cast<uint32_t>(write_descriptor_sets.size()),
											write_descriptor_sets.data(), 0, nullptr);

}

void ComputeParticles::create_compute_pipeline()
{

	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PushConstantCompute);

	VkPipelineLayoutCreateInfo compute_pipeline_layout_create_info{};
	compute_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	compute_pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(compute_descriptor_set_layouts.size());
	compute_pipeline_layout_create_info.pushConstantRangeCount = 1;
	compute_pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;
	compute_pipeline_layout_create_info.pSetLayouts = compute_descriptor_set_layouts.data();

	VkResult result = vkCreatePipelineLayout(context->device, &compute_pipeline_layout_create_info, nullptr,
															&compute_pipeline_layout);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a compute pipeline layout!");

	}

	// create pipeline 
	// build shader modules to link to graphics pipeline
	compile_shaders();

	auto vertex_shader_code = read_file("../Resources/Shader/spv/particle_integration.comp.spv");
	auto fragment_shader_code = read_file("../Resources/Shader/spv/particle_simulation.comp.spv");

	// build shader modules to link to graphics pipeline
	VkShaderModule compute_shader_module_simulation = create_shader_module(context->device, vertex_shader_code);
	VkShaderModule compute_shader_module_integration = create_shader_module(context->device, fragment_shader_code);

	// Specialization constant for workgroup size
	std::array<VkSpecializationMapEntry, 3> specEntries;
	
	specEntries[0].constantID = 0;
	specEntries[0].size = sizeof(context->spec_data.specWorkGroupSizeX);
	specEntries[0].offset = 0;

	specEntries[1].constantID = 1;
	specEntries[1].size = sizeof(context->spec_data.specWorkGroupSizeY);
	specEntries[1].offset = offsetof(SpecializationData, specWorkGroupSizeY);

	specEntries[2].constantID = 2;
	specEntries[2].size = sizeof(context->spec_data.specWorkGroupSizeZ);
	specEntries[2].offset = offsetof(SpecializationData, specWorkGroupSizeZ);
		
	VkSpecializationInfo specInfo{};
	specInfo.dataSize = sizeof(context->spec_data);
	specInfo.mapEntryCount = static_cast<uint32_t>(specEntries.size());
	specInfo.pMapEntries = specEntries.data();
	specInfo.pData = &context->spec_data;

	VkPipelineShaderStageCreateInfo compute_shader_integrate_create_info{};
	compute_shader_integrate_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	compute_shader_integrate_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	compute_shader_integrate_create_info.module = compute_shader_module_integration;
	compute_shader_integrate_create_info.pSpecializationInfo = &specInfo;
	compute_shader_integrate_create_info.pName = "main";

	// -- COMPUTE PIPELINE CREATION --
	VkComputePipelineCreateInfo compute_pipeline_integrate_create_info{};
	compute_pipeline_integrate_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	compute_pipeline_integrate_create_info.stage = compute_shader_integrate_create_info;
	compute_pipeline_integrate_create_info.layout = compute_pipeline_layout;
	compute_pipeline_integrate_create_info.flags = 0;
	// create compute pipeline 
	result = vkCreateComputePipelines(context->device, VK_NULL_HANDLE, 1, &compute_pipeline_integrate_create_info, nullptr, &compute_pipeline_integration);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a compute pipeline!");

	}

	VkPipelineShaderStageCreateInfo compute_shader_simulation_create_info{};
	compute_shader_simulation_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	compute_shader_simulation_create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	compute_shader_simulation_create_info.module = compute_shader_module_simulation;
	compute_shader_simulation_create_info.pSpecializationInfo = &specInfo;
	compute_shader_simulation_create_info.pName = "main";

	// -- COMPUTE PIPELINE CREATION --
	VkComputePipelineCreateInfo compute_pipeline_simulation_create_info{};
	compute_pipeline_simulation_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	compute_pipeline_simulation_create_info.stage = compute_shader_simulation_create_info;
	compute_pipeline_simulation_create_info.layout = compute_pipeline_layout;
	compute_pipeline_simulation_create_info.flags = 0;

	compute_shader_simulation_create_info.module = compute_shader_module_simulation;
	result = vkCreateComputePipelines(context->device, VK_NULL_HANDLE, 1, &compute_pipeline_simulation_create_info, nullptr, &compute_pipeline_simulation);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a compute pipeline!");

	}

	// Destroy shader modules, no longer needed after pipeline created
	vkDestroyShaderModule(context->device, compute_shader_module_simulation, nullptr);
	vkDestroyShaderModule(context->device, compute_shader_module_integration, nullptr);

}

void ComputeParticles::create_command_buffers()
{

	if(context->workGroupTesting){ 
		if (compute_timing_tester == nullptr) {
			// initalize our testing class for finding perfekt workgroup size
			compute_timing_tester = new ComputeTimingTester;
			compute_timing_tester->initialize(context);
			compute_timing_tester_initalized = true;
		}
		compute_timing_tester->next(); 
	}
	else {
		if (compute_timing_tester_initalized) {

			delete compute_timing_tester;
			compute_timing_tester = nullptr;
			compute_timing_tester_initalized = false;

		}
	}

	if (context->computeUniformChanged) {

		vkDeviceWaitIdle(context->device);
		context->computeUniformChanged = false;
		/*push_constant.firstVectorFieldStrength	= context->vectorFieldVelocityStrength[0];
		push_constant.secondVectorFieldStrength = context->vectorFieldVelocityStrength[1];
		push_constant.thirdVectorFieldStrength	= context->vectorFieldVelocityStrength[2];
		push_constant.particleVelocity			= context->particleVelocity;*/

	}

	command_buffer = resource_manager->startSingleTimeCmdBuffer();

	resource_manager->cmdResetQueryPool(command_buffer);
	// we have reset the pool; hence start by 0 
	uint32_t query = 0;

	// Start capture time of compute pass 
	resource_manager->cmdWriteTimeStamp(command_buffer, query++);

	if (context->queueFamilyIndices.graphics_family != context->queueFamilyIndices.compute_family) {

		VkBufferMemoryBarrier buffer_barrier =
		{
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			nullptr,
			0,
			VK_ACCESS_SHADER_WRITE_BIT,
			context->queueFamilyIndices.graphics_family,
			context->queueFamilyIndices.compute_family,
			context->storage_buffer_read_results[context->currentSwapchainIndex].buffer,
			0,
			(context->numParticlesX * context->numParticlesY * context->numParticlesZ) * sizeof(Particle)
		};

		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0, nullptr,
			1, &buffer_barrier,
			0, nullptr);

	}

	VkBufferMemoryBarrier buffer_barrier =
	{
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_SHADER_WRITE_BIT,
		0,
		0,
		context->storage_buffer_read_results[context->currentSwapchainIndex].buffer,
		0,
		(context->numParticlesX * context->numParticlesY * context->numParticlesZ) * sizeof(Particle)
	};

	vkCmdPipelineBarrier(
		command_buffer,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		0, nullptr,
		1, &buffer_barrier,
		0, nullptr);

	// First pass: calculate our movement
	// ------------------------------------
	// Start capture time of pipeline statistics of simulation pass 
	resource_manager->cmdWriteTimeStamp(command_buffer, query++);

	push_constant = {	
						{	
							context->numParticlesX,
							context->numParticlesY,
							context->numParticlesZ,
							context->particleType
						},

						{	
							context->vector_field_dim.x,
							context->vector_field_dim.y,
							context->vector_field_dim.z, 
							last_delta_t
						},

						{
							context->vectorFieldVelocityStrength[0],
							context->vectorFieldVelocityStrength[1],
							context->vectorFieldVelocityStrength[2],
							context->particleVelocity
						},

						glm::vec4(camera->get_camera_direction(),0.0f),

						context->particleModel

					};

	vkCmdPushConstants(command_buffer,
		compute_pipeline_layout,
		VK_SHADER_STAGE_COMPUTE_BIT,
		0,
		sizeof(PushConstantCompute),
		&push_constant);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
						compute_pipeline_simulation);

	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, 
							compute_pipeline_layout, 0, 1, &compute_descriptor_sets[context->currentSwapchainIndex],
							0,0);

	uint32_t workGroupCountX = std::max((context->numParticlesX + context->spec_data.specWorkGroupSizeX - 1) / context->spec_data.specWorkGroupSizeX, 1U);
	uint32_t workGroupCountY = std::max((context->numParticlesY + context->spec_data.specWorkGroupSizeY - 1) / context->spec_data.specWorkGroupSizeY, 1U);
	uint32_t workGroupCountZ = std::max((context->numParticlesZ + context->spec_data.specWorkGroupSizeZ - 1) / context->spec_data.specWorkGroupSizeZ, 1U);

	vkCmdDispatch(command_buffer, workGroupCountX, workGroupCountY, workGroupCountZ);

	// End of capturing time pipeline statistics of simulation pass 
	resource_manager->cmdWriteTimeStamp(command_buffer, query++);

	VkBufferMemoryBarrier buffer_barrier_transition_buffer_from_write_to_read{};
	buffer_barrier_transition_buffer_from_write_to_read.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	buffer_barrier_transition_buffer_from_write_to_read.pNext = nullptr;
	buffer_barrier_transition_buffer_from_write_to_read.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	buffer_barrier_transition_buffer_from_write_to_read.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	buffer_barrier_transition_buffer_from_write_to_read.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	buffer_barrier_transition_buffer_from_write_to_read.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	buffer_barrier_transition_buffer_from_write_to_read.buffer = storage_buffer_computation.buffer;
	buffer_barrier_transition_buffer_from_write_to_read.offset = 0;
	buffer_barrier_transition_buffer_from_write_to_read.size = (context->numParticlesX * context->numParticlesY * context->numParticlesZ) * sizeof(Particle);

	vkCmdPipelineBarrier(command_buffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		0, nullptr,
		1, &buffer_barrier_transition_buffer_from_write_to_read,
		0, nullptr);

	// Second Pass : now we can integrate :)
	// start capturing time of integration pass 
	resource_manager->cmdWriteTimeStamp(command_buffer, query++);
	vkCmdPushConstants(command_buffer,
		compute_pipeline_layout,
		VK_SHADER_STAGE_COMPUTE_BIT,
		0,
		sizeof(PushConstantCompute),
		&push_constant);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
						compute_pipeline_integration);

	vkCmdDispatch(command_buffer, workGroupCountX, workGroupCountY, workGroupCountZ);

	// end capturing time of íntegration pass 

	// region of data to copy from and to
	VkBufferCopy buffer_copy_region{};
	buffer_copy_region.srcOffset = 0;
	buffer_copy_region.dstOffset = 0;
	buffer_copy_region.size = (context->numParticlesX * context->numParticlesY * context->numParticlesZ) * sizeof(Particle);

	VkBufferMemoryBarrier buffer_barrier_read_only_after_write{};
	buffer_barrier_read_only_after_write.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	buffer_barrier_read_only_after_write.pNext = nullptr;
	buffer_barrier_read_only_after_write.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	buffer_barrier_read_only_after_write.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	buffer_barrier_read_only_after_write.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	buffer_barrier_read_only_after_write.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	buffer_barrier_read_only_after_write.buffer = storage_buffer_computation.buffer;
	buffer_barrier_read_only_after_write.offset = 0;
	buffer_barrier_read_only_after_write.size = context->numParticlesX * 
												context->numParticlesY * 
												context->numParticlesZ * sizeof(Particle);

	vkCmdPipelineBarrier(command_buffer,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		0,
		0, nullptr,
		1, &buffer_barrier_read_only_after_write,
		0, nullptr);

	// command to copy src buffer to dst buffer
	vkCmdCopyBuffer(command_buffer, storage_buffer_computation.buffer, context->storage_buffer_read_results[context->currentSwapchainIndex].buffer, 1, &buffer_copy_region);

	resource_manager->cmdWriteTimeStamp(command_buffer, query++);

	
	if (context->queueFamilyIndices.graphics_family != context->queueFamilyIndices.compute_family) {

		VkBufferMemoryBarrier buffer_barrier_storage_buffer_read{};
		buffer_barrier_storage_buffer_read.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		buffer_barrier_storage_buffer_read.pNext = nullptr;
		buffer_barrier_storage_buffer_read.srcQueueFamilyIndex = context->queueFamilyIndices.compute_family;
		buffer_barrier_storage_buffer_read.dstQueueFamilyIndex = context->queueFamilyIndices.graphics_family;
		buffer_barrier_storage_buffer_read.buffer = context->storage_buffer_read_results[context->currentSwapchainIndex].buffer;
		buffer_barrier_storage_buffer_read.offset = 0;
		buffer_barrier_storage_buffer_read.size = (context->numParticlesX * context->numParticlesY * context->numParticlesZ) * sizeof(Particle);

		vkCmdPipelineBarrier(command_buffer,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			0,
			0, nullptr,
			1, &buffer_barrier_storage_buffer_read,
			0, nullptr);

	}

	HostTimer timer;

	std::vector<VkPipelineStageFlags>   waitStages = {	VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
	//VkPipelineStageFlags pipeline_flags = ;
	SubmitSynchronizationInfo sync_info{};
	sync_info.pWaitDstStageMask = nullptr;
	sync_info.waitSemaphoreCount = 0; 
	sync_info.signalSemaphoreCount = 1;
	sync_info.pSignalSemaphore = &context->renderComputeSync[context->currentSwapchainIndex];
	
	// capture time of compute pipeline end 
	resource_manager->cmdWriteTimeStamp(command_buffer, query++);

	resource_manager->cmdBufEndAndSubmit(&command_buffer, 1, false, true);

	//resource_manager->cmdBufEndAndSubmitSynchronized(&command_buffer, 1, sync_info, true);

	bool queryPoolIsReady = resource_manager->getQueryResults(queryResults);

	if (queryPoolIsReady) {

		uint64_t time_simulation_stage_nanoseconds	=	(queryResults[2] - queryResults[1])	* physicalDeviceProps.limits.timestampPeriod;
		uint64_t time_integration_stage_nanoseconds =	(queryResults[4] - queryResults[3])	* physicalDeviceProps.limits.timestampPeriod;
		uint64_t time_compute_pass_nanoseconds		=	(queryResults[5] - queryResults[0])	* physicalDeviceProps.limits.timestampPeriod;

		context->time_simulation_stage_ms	= static_cast<float>(time_simulation_stage_nanoseconds)		/ 1000000.f;
		context->time_integration_stage_ms	= static_cast<float>(time_integration_stage_nanoseconds)	/ 1000000.f;
		context->time_compute_pass_ms		= static_cast<float>(time_compute_pass_nanoseconds)			/ 1000000.f;
	}

	// ----- needed for delta time numerical integration
	last_delta_t = static_cast<float>(timer.elapsed() * 1000);

}

void ComputeParticles::create_vector_fields()
{
	glm::mat4 vectorFieldModel	= glm::scale(glm::mat4(1.f), glm::vec3(	context->particleAreaOfInfluenceX,
																		context->particleAreaOfInfluenceY, 
																		context->particleAreaOfInfluenceZ));

	vectorFieldModel			= glm::translate(vectorFieldModel, context->particleTranslation);

	vector_field_images.resize(NUM_VECTOR_FIELDS);
	vector_field_samplers.resize(NUM_VECTOR_FIELDS);

	for (int i = 0; i < NUM_VECTOR_FIELDS; i++) {

		vectorFields.emplace_back(	context->vector_field_dim.x,
									context->vector_field_dim.y,
									context->vector_field_dim.z,
									context->vectorFieldStrategies[i],
									vectorFieldModel);

		VkImageCreateInfo vector_field_image_create_info{};
		vector_field_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		vector_field_image_create_info.pNext = nullptr;
		vector_field_image_create_info.imageType = VK_IMAGE_TYPE_3D;
		vector_field_image_create_info.format = vectorFieldImageFormat;
		vector_field_image_create_info.extent.width = context->vector_field_dim.x;
		vector_field_image_create_info.extent.height = context->vector_field_dim.y;
		vector_field_image_create_info.extent.depth = context->vector_field_dim.z;
		vector_field_image_create_info.mipLevels = 1;
		vector_field_image_create_info.arrayLayers = 1;
		vector_field_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		vector_field_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		vector_field_image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		vector_field_image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vector_field_image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		createImage(context->device, context->allocator, vector_field_image_create_info, vector_field_images[i]);

		VkImageViewCreateInfo vector_field_image_view_create_info{};
		vector_field_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vector_field_image_view_create_info.pNext = nullptr;
		vector_field_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
		vector_field_image_view_create_info.image = vector_field_images[i].image;
		vector_field_image_view_create_info.format = vectorFieldImageFormat;
		vector_field_image_view_create_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		vector_field_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vector_field_image_view_create_info.subresourceRange.baseMipLevel = 0;
		vector_field_image_view_create_info.subresourceRange.baseArrayLayer = 0;
		vector_field_image_view_create_info.subresourceRange.layerCount = 1;
		vector_field_image_view_create_info.subresourceRange.levelCount = 1;

		createImageView(context->device, vector_field_image_view_create_info, vector_field_images[i].view);

		VkCommandBuffer cmdBuffer = resource_manager->startSingleTimeCmdBuffer();

		transition_image_layout_for_command_buffer(cmdBuffer, vector_field_images[i].image, VK_IMAGE_LAYOUT_UNDEFINED,
														VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);

		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.compareOp = VK_COMPARE_OP_NEVER;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = 0.0f;
		sampler_info.maxAnisotropy = 1.0;
		sampler_info.anisotropyEnable = VK_FALSE;
		sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		sampler_info.unnormalizedCoordinates = VK_FALSE;

		vkCreateSampler(context->device, &sampler_info, nullptr, &vector_field_samplers[i]);

		Buffer intermediate_vector_field_buffer{};

		VkDeviceSize dataSize = vectorFields[i].get_velocity_data().size() * sizeof(glm::vec4);
		createBufferWithAllocator(context->device, context->allocator, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY,
			intermediate_vector_field_buffer.buffer, intermediate_vector_field_buffer.allocation);

		// Write data
		writeVectorToBuffer(context->device, context->allocator, intermediate_vector_field_buffer, vectorFields[i].get_velocity_data());
		
		// Setup buffer copy regions
		VkBufferImageCopy bufferCopyRegion{};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = context->vector_field_dim.x;
		bufferCopyRegion.imageExtent.height = context->vector_field_dim.y;
		bufferCopyRegion.imageExtent.depth = context->vector_field_dim.z;

		vkCmdCopyBufferToImage(cmdBuffer, intermediate_vector_field_buffer.buffer, vector_field_images[i].image,
										VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
										&bufferCopyRegion);

		transition_image_layout_for_command_buffer(cmdBuffer, vector_field_images[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);


		resource_manager->endSingleTimeCmdBuffer(cmdBuffer);

		destroyBuffer(context->device, context->allocator, intermediate_vector_field_buffer);

	}


}

/**
	ASSUMPTIONS on texture layouts:
	----- Position, velocity and color texture: values in unit cube [0,1]
	----- load with 16 bit precision
*/
void ComputeParticles::create_particle_buffer()
{
	
	particles = {	context->numParticlesX, context->numParticlesY, context->numParticlesZ,
					context->particleModel};

	resource_manager->immediateCreateAndUploadDeviceBuffer<Particle>(storage_buffer_computation,
																	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
																	VK_BUFFER_USAGE_TRANSFER_SRC_BIT | 
																	VK_BUFFER_USAGE_TRANSFER_DST_BIT 
																	, particles.get_data());

	context->storage_buffer_read_results.resize(3);

	for (Buffer& read_buffer : context->storage_buffer_read_results) {

		resource_manager->immediateCreateAndUploadDeviceBuffer<Particle>(read_buffer,
											VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
											VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
											VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
											VK_BUFFER_USAGE_TRANSFER_SRC_BIT
											, particles.get_data());
	}

	VkCommandBuffer command_buffer = resource_manager->startSingleTimeCmdBuffer();
	for (Buffer& read_buffer : context->storage_buffer_read_results) {

		VkBufferMemoryBarrier buffer_barrier_read_results{};
		buffer_barrier_read_results.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		buffer_barrier_read_results.pNext = nullptr;
		buffer_barrier_read_results.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		buffer_barrier_read_results.dstAccessMask = 0;
		buffer_barrier_read_results.srcQueueFamilyIndex = context->queueFamilyIndices.graphics_family;
		buffer_barrier_read_results.dstQueueFamilyIndex = context->queueFamilyIndices.compute_family;
		buffer_barrier_read_results.buffer = read_buffer.buffer;
		buffer_barrier_read_results.offset = 0;
		buffer_barrier_read_results.size = (context->numParticlesX * 
											context->numParticlesY * 
											context->numParticlesZ) * sizeof(Particle);


		vkCmdPipelineBarrier(command_buffer,
							VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
							VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
							0,
							0, nullptr,
							1, &buffer_barrier_read_results,
							0, nullptr);
		

	}

	resource_manager->endSingleTimeCmdBuffer(command_buffer);

}

void ComputeParticles::create_uniform_buffers()
{

	std::vector<UboCompute> c_uniform = {ubo_compute};
	resource_manager->immediateCreateAndUploadDeviceBuffer<UboCompute>(compute_uniform_buffer,
											VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
											, c_uniform);

}

void ComputeParticles::create_descriptor_pool()
{
	
	VkDescriptorPoolSize pool_image_sampler_size{};
	pool_image_sampler_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_image_sampler_size.descriptorCount = static_cast<uint32_t>(NUM_VECTOR_FIELDS);

	VkDescriptorPoolSize pool_storage_buffer_size{};
	pool_storage_buffer_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pool_storage_buffer_size.descriptorCount = static_cast<uint32_t>(1);

	VkDescriptorPoolSize pool_uniform_buffer_size{};
	pool_uniform_buffer_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_uniform_buffer_size.descriptorCount = static_cast<uint32_t>(1);

	VkDescriptorPoolSize pool_object_description_buffer_size{};
	pool_object_description_buffer_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	pool_object_description_buffer_size.descriptorCount = static_cast<uint32_t>(1);

	// list of pool sizes 
	std::vector<VkDescriptorPoolSize> descriptor_pool_sizes = { pool_image_sampler_size,
																pool_storage_buffer_size,
																pool_uniform_buffer_size,
																pool_object_description_buffer_size };

	VkDescriptorPoolCreateInfo pool_create_info{};
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.maxSets = MAX_FRAME_DRAWS;												
	pool_create_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());
	pool_create_info.pPoolSizes = descriptor_pool_sizes.data();

	// create descriptor pool
	VkResult result = vkCreateDescriptorPool(context->device, &pool_create_info, nullptr, &compute_descriptor_pool);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a descriptor pool!");

	}

}

void ComputeParticles::clean_up_particle_buffer()
{

	// ----- for reading 
	for (Buffer read_buffer : context->storage_buffer_read_results) {
		destroyBuffer(context->device, context->allocator, read_buffer);
	}
	// ----- for simulation and writing to it 
	destroyBuffer(context->device, context->allocator, storage_buffer_computation);

}

void ComputeParticles::clean_up_vector_field()
{
	vectorFields.clear();
	for (int i = 0; i < NUM_VECTOR_FIELDS; i++) {

		destroyImage(context->device, context->allocator, vector_field_images[i]);
		vkDestroySampler(context->device, vector_field_samplers[i], nullptr);

	}

}
