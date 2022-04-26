#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <vector>
#include <memory>
#include <cstring>
#include <iostream>
#include <set>
#include <algorithm>
#include <array>
#include <stdlib.h>
#include <stdio.h>

#include "Common.h"

#include "Context.h"
#include "ResourceManager.h"
#include "Resources.h"
#include "Utilities.h"

#include "host_timer.h"

#include <CSVWriter.h>
#include <VectorField.h>
#include <Particles.h>
#include <ComputeTimingTester.h>
#include <Camera.h>

class ComputeParticles
{
public:

	ComputeParticles();
	ComputeParticles(ResourceManager* rm, Camera* camera);

	void reloadShaders();
	void compute();

	void clean_up();

	~ComputeParticles();

private:

	void create_command_buffers();
	void update_descriptor_set(uint32_t frame_index);

	void create_synchronization_objects();
	void create_descriptor_set();

	void create_compute_pipeline();

	void create_vector_fields();
	void create_particle_buffer();
	void create_uniform_buffers();

	void create_descriptor_pool();

	void clean_up_particle_buffer();
	void clean_up_vector_field();

	VkCommandBuffer command_buffer;

	Camera* camera;

	// Push constant structure for the compute
	struct PushConstantCompute
	{
		glm::vec4			numberAndTypeOfParticles;	// in X,Y,Z,  W = particle Type 
		glm::vec4			limitsAndTime;				// X,Y,Z = limits; w = delta_t
		glm::vec4			velocities;					// X,Y,Z = velocity of vector fields;, W = velocity particles
		glm::vec4			view;						// view vector
		glm::mat4			particleModel;

	};

	struct UboCompute {

		glm::vec4 data;

	};


	Particles particles;
	ParticlePositionLimits pos_limits;
	// ------------
	// Layout particle buffer:
	// access 3D-Position (x,y,z) like this:
	// Particles[z * numParticlesY * numParticlesX + y * numParticlesX + x]
	// ------------
	Buffer storage_buffer_computation;

	Context* context;
	ResourceManager* resource_manager;
	VkPhysicalDeviceProperties physicalDeviceProps;

	VkPipelineLayout compute_pipeline_layout;
	VkDescriptorPool compute_descriptor_pool;
	std::vector<VkDescriptorSetLayout> compute_descriptor_set_layouts;
	std::vector<VkDescriptorSet> compute_descriptor_sets;

	VkPipeline compute_pipeline_simulation;
	VkPipeline compute_pipeline_integration;

	VkPushConstantRange compute_push_constant_range_calculate;
	PushConstantCompute push_constant{};
	UboCompute ubo_compute;
	Buffer compute_uniform_buffer;

	// ----- Vector field ressources for accessing in shader 
	VkFormat vectorFieldImageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
	std::vector<VectorField>	vectorFields;
	std::vector<Image>			vector_field_images;
	std::vector<VkSampler>		vector_field_samplers;

	// ----- everything for the timing
	std::vector<uint64_t> queryResults;
	float last_delta_t = 0;
	bool compute_timing_tester_initalized = false;
	ComputeTimingTester* compute_timing_tester;

};

