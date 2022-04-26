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

#include "Window.h"
#include "Utilities.h"
#include "Camera.h"

// all IMGUI stuff


#include "ComputeParticles.h"
#include <ComputeTimingTester.h>
#include "RenderSystem.h"
#include "SceneLoadInfo.h"
#include <RtPhysics.h>
#include <ClassicPhysics.h>
#include <DescriptorSetManager.h>

struct AppInput
{
	bool triggerShaderReload;
};

class App
{
public:

	App();

	void runFrame(AppInput input);

	//int init(std::shared_ptr<MyWindow> window, glm::vec3 eye, float near_plane, float far_plane,
	//	glm::vec3 light_dir, glm::vec3 view_dir);

	int init(std::shared_ptr<Window> window, Camera* camera, glm::vec3 light_dir, SceneLoadInfo loadInfo);

	void clean_up();

	~App();

private:

	Context*			context;
	ResourceManager*	rm_grahics;
	ResourceManager*	rm_compute;
	Camera*				camera;

	VkInstance instance;
	VkPhysicalDeviceProperties device_properties;
	VkExtent2D swap_chain_extent;
	VkSurfaceKHR surface;
	std::shared_ptr<Window> window;

	// -- debugging
	VkDebugUtilsMessengerEXT debug_messenger;

	// Deferred renderer
	RenderSystem*			renderSystem;
	RtPhysics*				physicsSystem;
	ClassicPhysics*			classicPhysicsSystem;
	Scene*					scene;
	DescriptorSetManager*	descriptorSetManager;

	// Particle simulation
	ComputeParticles* computeParticles;


	// Maybe DELETE, replaced by context
	MainDevice_ MainDevice;
	VkQueue graphics_queue;
	VkQueue presentation_queue;
	VkQueue compute_queue;
	VkSwapchainKHR swapchain;
	VkFormat swap_chain_image_format;
	std::vector<SwapChainImage> swap_chain_images;

	VkDescriptorPool gui_descriptor_pool;

	// Vulkan specific functions
	void create_instance();
	void create_logical_device();
	void create_surface();
	void create_swap_chain();
	void get_physical_device();
	void clean_up_swapchain();

	//  Other initialiser functions
	void createContext();
	void createResourceManagers();


};