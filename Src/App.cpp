#include "App.h"
#include "ResourceManager.h"
#include "PipelineToolKit.h"
#include "NoMovement.h"

App::App()
{
}



void App::runFrame(AppInput input)
{
	if (context->imagesInFlight[context->currentSwapchainIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(context->device, 1, &context->imagesInFlight[context->currentSwapchainIndex], VK_TRUE, UINT64_MAX);
	}
	// Reset resource pools
	rm_grahics->update();
	rm_compute->update();

	if (g_reset_grid.val.v_bool)
	{
		scene->resetDynamicGeometry();
		context->totalFrameIndex = 0;
	}

	if (input.triggerShaderReload || context->computeShaderChanged)
	{
		renderSystem->reloadShaders();
		computeParticles->reloadShaders();

		if (g_enable_classic_physics.val.v_bool)
		{
			classicPhysicsSystem->reloadShaders();
		}
		else
		{
			physicsSystem->reloadShaders();
		}

		context->computeShaderChanged = false;
		context->totalFrameIndex = 0;
		std::cout << "Reloaded Shaders!\n";
	}


	//vkDeviceWaitIdle(context->device);

	scene->update(camera);
	computeParticles->compute();

	if (g_enable_classic_physics.val.v_bool)
	{
		classicPhysicsSystem->runSimulation(scene);
	}
	else
	{
		physicsSystem->runSimulation(scene);
	}

	renderSystem->render();

	/*VkResult result = vkDeviceWaitIdle(context->device);
	ASSERT_VULKAN(result)*/

	//vkDeviceWaitIdle(context->device);
}


int App::init(std::shared_ptr<Window> window, Camera* camera, glm::vec3 light_dir, SceneLoadInfo loadInfo)
{
	this->window = window;
	this->camera = camera;
	
	try {

		create_instance();
		create_surface();
		get_physical_device();
		create_logical_device();
		create_swap_chain();

		// Initialisation
		createContext();
		createResourceManagers();

		// Create Descriptor Sets
		descriptorSetManager = new DescriptorSetManager(context);


		// Create and load scene
		scene = new Scene(rm_grahics, descriptorSetManager);
		scene->load(loadInfo);

		// Create render system
		renderSystem = new RenderSystem(rm_grahics, camera, descriptorSetManager, scene);

		// Create particle system
		computeParticles = new ComputeParticles(rm_compute, camera);

		// Create physics system
		physicsSystem = new RtPhysics(rm_compute, descriptorSetManager);
		
		// Create classic physics system
		classicPhysicsSystem = new ClassicPhysics(rm_compute, descriptorSetManager);

	}
	catch (const std::runtime_error& e) {

		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;

	}

	return EXIT_SUCCESS;

}

void App::clean_up()
{
	for (int i = 0; i < NUM_VECTOR_FIELDS; i++) {

		if (context->vectorFieldStrategies[i] != nullptr) {
			delete context->vectorFieldStrategies[i];
			context->vectorFieldStrategies[i] = 0;
		}

	}

	PFN_vkDestroyDebugUtilsMessengerEXT pvkDestroyDebugUtilsMessengerEXT =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	// wait until no actions being run on device before destroying
	vkDeviceWaitIdle(MainDevice.logical_device);

	clean_up_swapchain();

	if (ENABLE_VALIDATION_LAYERS) {
		pvkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, NULL);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(MainDevice.logical_device, nullptr);
	vkDestroyInstance(instance, nullptr);

}

//////////////////////////////////////////////////////////////////////////////////////////////////// App init stuff:


void App::createContext()
{
	this->context					= new Context();
	context->window					= this->window->get_window();
	context->device					= MainDevice.logical_device;
	context->physDevice				= MainDevice.physical_device;
	context->instance				= instance;
	context->currentSwapchainIndex	= 0;
	context->width					= 1200; // Temporary
	context->height					= 768;
	context->swapChainImages		= swap_chain_images;
	context->swapChainImageFormat	= swap_chain_image_format;
	context->swapchain				= swapchain;
	context->queueFamilyIndices		= get_queue_families(context->physDevice, surface);
	context->allocator				= new Allocator(context->device, context->physDevice, instance);

	context->particleAreaOfInfluenceX		= 0.1f;
	context->particleAreaOfInfluenceY		= 0.1f;
	context->particleAreaOfInfluenceZ		= 0.1f;
	context->totalFrameIndex				= 0;

	context->particleTranslation			= glm::vec3(0.0f, -12.f, 0.0f);

	context->particleModel					= glm::translate(	glm::scale(glm::mat4(1.f),glm::vec3(context->particleAreaOfInfluenceX,
																									context->particleAreaOfInfluenceY,
																									context->particleAreaOfInfluenceZ)),
																									context->particleTranslation);

	context->vector_field_dim.x					= 8;
	context->vector_field_dim.y					= 8;
	context->vector_field_dim.z					= 8;

	context->posLimits.maxPositionX				= context->vector_field_dim.x;
	context->posLimits.maxPositionY				= context->vector_field_dim.y;
	context->posLimits.maxPositionZ				= context->vector_field_dim.z;

	context->particleVelocity					= 1.f;

	context->particleType						= ParticleTypes::Bounce;

	context->vectorFieldTranslationDirections.resize(NUM_VECTOR_FIELDS);
	context->vectorFieldVelocityStrength.resize(NUM_VECTOR_FIELDS);
	context->vectorFieldStrategies.resize(NUM_VECTOR_FIELDS);

	context->vectorFieldTranslationDirections[0]	= glm::vec3(1.f, 0.0f, 0.0f);
	context->vectorFieldVelocityStrength[0]			= 0.1f;
	context->vectorFieldStrategies[0]				= new RotationMovement(0);
	
	context->vectorFieldTranslationDirections[1]	= glm::vec3(1.f, 0.0f, 0.0f);
	context->vectorFieldVelocityStrength[1]			= 0.1f;
	context->vectorFieldStrategies[1]				= new RotationMovement(1);

	context->vectorFieldTranslationDirections[2]	= glm::vec3(1.f, 0.0f, 0.0f);
	context->vectorFieldVelocityStrength[2]			= 0.1f;
	context->vectorFieldStrategies[2]				= new RotationMovement(2);

	context->workGroupTestingProgess			= 0.0f;
	context->workGroupTesting					= false;

	context->bestWorkGroupSizeX					= 1;
	context->bestWorkGroupSizeY					= 1;
	context->bestWorkGroupSizeZ					= 1;

	context->spec_data.specWorkGroupSizeX		= 64;
	context->spec_data.specWorkGroupSizeY		= 16;
	context->spec_data.specWorkGroupSizeZ		= 1;

	vkGetPhysicalDeviceProperties(context->physDevice, &context->physicalDeviceProps);

}


void App::createResourceManagers()
{
	QueueFamilyIndices indices = get_queue_families(MainDevice.physical_device, surface);
	rm_grahics = new ResourceManager(context, indices.graphics_family);
	rm_compute = new ResourceManager(context, indices.compute_family);
}

void App::clean_up_swapchain()
{

	for (auto image : swap_chain_images) {

		vkDestroyImageView(MainDevice.logical_device, image.image_view, nullptr);

	}

	vkDestroySwapchainKHR(MainDevice.logical_device, swapchain, nullptr);

}


App::~App()
{
}



//////////////////////////////////////////////////////////////////////////////////////////////////// Vulkan init stuff:





void App::create_instance()
{
	if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support()) {
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	// info about app
	// most data doesn't affect program; is for developer convenience
	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Epic Graphics";								// custom name of app
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);			// custom version of app
	app_info.pEngineName = "Cataglyphis Renderer";						// custom engine name
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 3);					// custom engine version 
	app_info.apiVersion = VK_API_VERSION_1_2;									// the vulkan version

	// creation info for a VkInstance
	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
	//add validation layers IF enabled to the creeate info struct
	if (ENABLE_VALIDATION_LAYERS) {

		uint32_t layerCount = 1;
		const char** layerNames = (const char**)malloc(sizeof(const char*) * layerCount);
		//layerNames[0] = "VK_LAYER_KHRONOS_validation";

		messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		messengerCreateInfo.pfnUserCallback = debugCallback;

		//create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengerCreateInfo;

		create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		create_info.ppEnabledLayerNames = validationLayers.data();

	}
	else {

		create_info.enabledLayerCount = 0;
		create_info.pNext = nullptr;

	}

	// create list to hold instance extensions
	std::vector<const char*> instance_extensions = std::vector<const char*>();

	//Setup extensions the instance will use 
	uint32_t glfw_extensions_count = 0;																													// GLFW may require multiple extensions
	const char** glfw_extensions;																																// Extensions passed as array of cstrings, so need pointer(array) to pointer
																																														// (the cstring)

	//set GLFW extensions
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

	// Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfw_extensions_count; i++) {

		instance_extensions.push_back(glfw_extensions[i]);

	}

	if (ENABLE_VALIDATION_LAYERS) {

		instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	}

	// check instance extensions supported
	if (!check_instance_extension_support(&instance_extensions)) {

		throw std::runtime_error("VkInstance does not support required extensions!");

	}

	create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
	create_info.ppEnabledExtensionNames = instance_extensions.data();

	// create instance 
	VkResult result = vkCreateInstance(&create_info, nullptr, &instance);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a Vulkan instance!");
	}

	if (ENABLE_VALIDATION_LAYERS) {

		PFN_vkCreateDebugUtilsMessengerEXT pvkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (pvkCreateDebugUtilsMessengerEXT(instance, &messengerCreateInfo, NULL, &debug_messenger) == VK_SUCCESS) {
			printf("created debug messenger\n");
		}

	}

}

void App::create_logical_device()
{

	// get the queue family indices for the chosen physical device
	QueueFamilyIndices indices = get_queue_families(MainDevice.physical_device, surface);

	// vector for queue creation information and set for family indices
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::set<int> queue_family_indices = { indices.graphics_family, indices.presentation_family, indices.compute_family };

	// Queue the logical device needs to create and info to do so (only 1 for now, will add more later!)
	for (int queue_family_index : queue_family_indices) {

		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_family_index;															// the index of the family to create a queue from
		queue_create_info.queueCount = 1;																											// number of queues to create
		float priority = 1.0f;
		queue_create_info.pQueuePriorities = &priority;																						//Vulkan needs to know how to handle multiple queues, so decide priority (1 = highest)

		queue_create_infos.push_back(queue_create_info);

	}

	// -- ALL EXTENSION WE NEED
	VkPhysicalDeviceFeatures usedFeatures = {};
	usedFeatures.samplerAnisotropy = VK_TRUE;
	usedFeatures.fragmentStoresAndAtomics = VK_TRUE;
	usedFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;
	usedFeatures.shaderInt64 = VK_TRUE;
	//usedFeatures.pipelineStatisticsQuery = VK_TRUE;


	VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shaderAtomicFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT };
	shaderAtomicFeatures.shaderBufferFloat32Atomics = true;
	shaderAtomicFeatures.shaderBufferFloat32AtomicAdd = true;

	VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
	rayQueryFeatures.pNext = &shaderAtomicFeatures;
	rayQueryFeatures.rayQuery = VK_TRUE;

	VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT   shader64Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT };
	shader64Features.shaderImageInt64Atomics = VK_TRUE;
	shader64Features.pNext = &rayQueryFeatures;

	VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{};
	indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	indexing_features.runtimeDescriptorArray = VK_TRUE;
	indexing_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	indexing_features.pNext = &shader64Features;

	VkPhysicalDeviceFeatures2 features2{};
	features2.pNext = &indexing_features;
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.features.samplerAnisotropy = VK_TRUE;
	features2.features.shaderInt64 = VK_TRUE;
	features2.features.multiDrawIndirect = VK_TRUE;
	//features2.features.pipelineStatisticsQuery = VK_TRUE;

	// -- NEEDED FOR QUERING THE DEVICE ADDRESS WHEN CREATING ACCELERATION STRUCTURES
	VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_features{};
	buffer_device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	buffer_device_address_features.pNext = &features2;
	buffer_device_address_features.bufferDeviceAddress = VK_TRUE;
	buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;
	buffer_device_address_features.bufferDeviceAddressMultiDevice = VK_FALSE;

	// --ENABLE RAY TRACING PIPELINE
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features{};
	ray_tracing_pipeline_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	ray_tracing_pipeline_features.pNext = &buffer_device_address_features;
	//ray_tracing_pipeline_features.pNext = &features2;
	ray_tracing_pipeline_features.rayTracingPipeline = VK_TRUE;

	// -- ENABLE ACCELERATION STRUCTURES
	VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{};
	acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	acceleration_structure_features.pNext = &ray_tracing_pipeline_features;
	acceleration_structure_features.accelerationStructure = VK_TRUE;
	//acceleration_structure_features.accelerationStructureCaptureReplay = VK_TRUE;
	acceleration_structure_features.accelerationStructureIndirectBuild = VK_FALSE;
	acceleration_structure_features.accelerationStructureHostCommands = VK_FALSE;
	acceleration_structure_features.descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE;

	// -- PREPARE FOR HAVING MORE EXTENSION BECAUSE WE NEED RAYTRACING CAPABILITIES
	std::vector<const char*> extensions(device_extensions);

	// COPY ALL NECESSARY EXTENSIONS FOR RAYTRACING TO THE EXTENSION
	extensions.insert(extensions.begin(), device_extensions_for_raytracing.begin(),
		device_extensions_for_raytracing.end());


	




	// information to create logical device (sometimes called "device") 
	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());		// number of queue create infos
	device_create_info.pQueueCreateInfos = queue_create_infos.data();														// list of queue create infos so device can create required queues
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());		// number of enabled logical device extensions
	device_create_info.ppEnabledExtensionNames = extensions.data();											// list of enabled logical device extensions 
	device_create_info.flags = 0;
	device_create_info.pEnabledFeatures = &usedFeatures;


	device_create_info.pNext = &acceleration_structure_features;
	//device_create_info.pNext = &features2;

	//// physical device features the logical device will be using 
	//VkPhysicalDeviceFeatures device_features{};
	//device_features.samplerAnisotropy = VK_TRUE;
	//device_create_info.pEnabledFeatures = &device_features;

	// create logical device for the given physical device
	VkResult result = vkCreateDevice(MainDevice.physical_device, &device_create_info, nullptr, &MainDevice.logical_device);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a logical device!");
	}

	//  Queues are created at the same time as the device...
	// So we want handle to queues
	// From given logical device of given queue family, of given queue index (0 since only one queue), place reference in given VkQueue
	vkGetDeviceQueue(MainDevice.logical_device, indices.graphics_family, 0, &graphics_queue);
	vkGetDeviceQueue(MainDevice.logical_device, indices.presentation_family, 0, &presentation_queue);
	vkGetDeviceQueue(MainDevice.logical_device, indices.compute_family, 0, &compute_queue);

}


void App::get_physical_device()
{

	// Enumerate physical devices the vkInstance can access
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

	// if no devices available, then none support of Vulkan
	if (device_count == 0) {
		throw std::runtime_error("Can not find GPU's that support Vulkan Instance!");
	}

	//Get list of physical devices 
	std::vector<VkPhysicalDevice> device_list(device_count);
	vkEnumeratePhysicalDevices(instance, &device_count, device_list.data());

	for (const auto& device : device_list) {

		if (check_device_suitable(device, surface)) {

			MainDevice.physical_device = device;
			break;

		}

	}

	// get properties of our new device
	vkGetPhysicalDeviceProperties(MainDevice.physical_device, &device_properties);

}

void App::create_surface()
{
	// create surface (creates a surface create info struct, runs the create surface function, returns result)
	VkResult result = glfwCreateWindowSurface(instance, window->get_window(), nullptr, &surface);

	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create a surface!");

}

void App::create_swap_chain()
{

	// get swap chain details so we can pick the best settings
	SwapChainDetails swap_chain_details = get_swapchain_details(MainDevice.physical_device, surface);

	// 1. choose best surface format
	// 2. choose best presentation mode 
	// 3. choose swap chain image resolution

	VkSurfaceFormatKHR surface_format = choose_best_surface_format(swap_chain_details.formats);
	VkPresentModeKHR present_mode = choose_best_presentation_mode(swap_chain_details.presentation_mode);
	VkExtent2D extent = choose_swap_extent(swap_chain_details.surface_capabilities, window->get_window());

	// how many images are in the swap chain; get 1 more than the minimum to allow tiple buffering
	uint32_t image_count = swap_chain_details.surface_capabilities.minImageCount + 1;

	// if maxImageCount == 0, then limitless
	if (swap_chain_details.surface_capabilities.maxImageCount > 0 &&
		swap_chain_details.surface_capabilities.maxImageCount < image_count) {

		image_count = swap_chain_details.surface_capabilities.maxImageCount;

	}

	VkSwapchainCreateInfoKHR swap_chain_create_info{};
	swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain_create_info.surface = surface;																											// swapchain surface
	swap_chain_create_info.imageFormat = surface_format.format;																		// swapchain format
	swap_chain_create_info.imageColorSpace = surface_format.colorSpace;														// swapchain color space
	swap_chain_create_info.presentMode = present_mode;																						// swapchain presentation mode 
	swap_chain_create_info.imageExtent = extent;																									// swapchain image extents
	swap_chain_create_info.minImageCount = image_count;																					// minimum images in swapchain
	swap_chain_create_info.imageArrayLayers = 1;																									// number of layers for each image in chain
	swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		| VK_IMAGE_USAGE_TRANSFER_DST_BIT;										// what attachment images will be used as 
	swap_chain_create_info.preTransform = swap_chain_details.surface_capabilities.currentTransform;	// transform to perform on swap chain images
	swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;								// dont do blending; everything opaque
	swap_chain_create_info.clipped = VK_TRUE;																											// of course activate clipping ! :) 

	// get queue family indices
	QueueFamilyIndices indices = get_queue_families(MainDevice.physical_device, surface);

	// if graphics and presentation families are different then swapchain must let images be shared between families
	if (indices.graphics_family != indices.presentation_family) {

		uint32_t queue_family_indices[] = {
						(uint32_t)indices.graphics_family,
						(uint32_t)indices.presentation_family
		};

		swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;									// image share handling
		swap_chain_create_info.queueFamilyIndexCount = 2;																					// number of queues to share images between
		swap_chain_create_info.pQueueFamilyIndices = queue_family_indices;														// array of queues to share between 

	}
	else {

		swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swap_chain_create_info.queueFamilyIndexCount = 0;
		swap_chain_create_info.pQueueFamilyIndices = nullptr;

	}

	// if old swap chain been destroyed and this one replaces it then link old one to quickly hand over responsibilities
	swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;

	// create swap chain 
	VkResult result = vkCreateSwapchainKHR(MainDevice.logical_device, &swap_chain_create_info, nullptr, &swapchain);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed create swapchain!");

	}

	// store for later reference
	swap_chain_image_format = surface_format.format;
	swap_chain_extent = extent;

	// get swapchain images (first count, then values)
	uint32_t swapchain_image_count;
	vkGetSwapchainImagesKHR(MainDevice.logical_device, swapchain, &swapchain_image_count, nullptr);
	std::vector<VkImage> images(swapchain_image_count);
	vkGetSwapchainImagesKHR(MainDevice.logical_device, swapchain, &swapchain_image_count, images.data());

	swap_chain_images.clear();
	//swap_chain_images.resize(swapchain_image_count);

	for (size_t i = 0; i < images.size(); i++) {

		VkImage image = images[static_cast<uint32_t>(i)];
		// store image handle
		SwapChainImage swap_chain_image{};
		swap_chain_image.image = image;

		VkImageViewCreateInfo view_create_info = vk_default::renderTargetImageViewCreateInfo();
		view_create_info.image = image;
		view_create_info.format = swap_chain_image_format;
		view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createImageView(MainDevice.logical_device, view_create_info, swap_chain_image.image_view);

		// add to swapchain image list 
		swap_chain_images.push_back(swap_chain_image);

	}

}

