#include "ImGuiRenderStage.h"
#include "Vertex.h"
#include "PipelineToolKit.h"
#include "Utilities.h"
#include "ImGuiSubpass.h"
#include "TranslationMovement.h"
#include "RotationMovement.h"
#include "NoMovement.h"
#include "Explosion.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"



ImGuiRenderStage::ImGuiRenderStage(ResourceManager* rm, DescriptorSet* descriptorSet, RenderAttachmentInfo attachementInfo, uint32_t subpassIndex)
{
    this->context = rm->context;
	this->rm = rm;
	this->isInit = false;
    this->subpass = new ImGuiSubpass(attachementInfo);
	this->subpassIndex = subpassIndex;


	// Init stuff
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	float size_pixels = 18;
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/Roboto-Medium.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/Cousine-Regular.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/DroidSans.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/Karla-Regular.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/ProggyClean.ttf", size_pixels);
	io.Fonts->AddFontFromFileTTF("../ExternalLib/IMGUI/misc/fonts/ProggyTiny.ttf", size_pixels);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGui_ImplGlfw_InitForVulkan(context->window, true);

	// Create Descriptor Pool
	VkDescriptorPoolSize gui_pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo gui_pool_info = {};
	gui_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	gui_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	gui_pool_info.maxSets = 1000 * IM_ARRAYSIZE(gui_pool_sizes);
	gui_pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(gui_pool_sizes);
	gui_pool_info.pPoolSizes = gui_pool_sizes;

	VkResult result = vkCreateDescriptorPool(context->device, &gui_pool_info, nullptr, &gui_descriptor_pool);

	if (result != VK_SUCCESS) {

		throw std::runtime_error("Failed to create a gui descriptor pool!");
	}


	
}

void ImGuiRenderStage::destroyPipeline()
{

	// clean up of GUI stuff
	ImGui_ImplVulkan_Shutdown();

}

ImGuiRenderStage::~ImGuiRenderStage()
{
}

void ImGuiRenderStage::destroyShaders()
{
}

void ImGuiRenderStage::compileShaders()
{
}

void ImGuiRenderStage::createPipeline(VkRenderPass renderPass)
{
	VkQueue queue;
	vkGetDeviceQueue(context->device, context->queueFamilyIndices.graphics_family, 0, &queue);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = context->instance;
	init_info.PhysicalDevice = context->physDevice;
	init_info.Device = context->device;
	init_info.QueueFamily = context->queueFamilyIndices.graphics_family;
	init_info.Queue = queue;
	init_info.DescriptorPool = gui_descriptor_pool;
	init_info.PipelineCache = VK_NULL_HANDLE;																					// we do not need those 
	init_info.MinImageCount = MAX_FRAME_DRAWS;
	init_info.ImageCount = MAX_FRAME_DRAWS;
	init_info.Allocator = VK_NULL_HANDLE;
	init_info.CheckVkResultFn = VK_NULL_HANDLE;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Subpass = subpassIndex;

	ImGui_ImplVulkan_Init(&init_info, renderPass);

	// Upload stuff
	VkCommandBuffer command_buffer = rm->startSingleTimeCmdBuffer();
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	rm->endSingleTimeCmdBuffer(command_buffer);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool ImGuiRenderStage::newWorkgroupSizesAreSuitable(int workGroupSizeX, int workGroupSizeY, int workGroupSizeZ) {

	if (workGroupSizeX > context->compute_limits.maxComputeWorkGroupSize[0] || 
		workGroupSizeY > context->compute_limits.maxComputeWorkGroupSize[1] || 
		workGroupSizeZ > context->compute_limits.maxComputeWorkGroupSize[2])
	{
		return false;
	}

	int numWorkGroupX = (context->numParticlesX + workGroupSizeX - 1) / workGroupSizeX;
	int numWorkGroupY = (context->numParticlesY + workGroupSizeY - 1) / workGroupSizeY;
	int numWorkGroupZ = (context->numParticlesZ + workGroupSizeZ - 1) / workGroupSizeZ;
	
	if (numWorkGroupX * numWorkGroupY * numWorkGroupZ > context->compute_limits.maxComputeWorkGroupInvocations) return false;
	
	return true;

}

bool ImGuiRenderStage::newNumParticlesAreSuitable(int numberParticlesX, int numberParticlesY, int numberParticlesZ)
{

	uint32_t workGroupCountX = std::max((context->numParticlesX + context->spec_data.specWorkGroupSizeX - 1) / context->spec_data.specWorkGroupSizeX, 1U);
	uint32_t workGroupCountY = std::max((context->numParticlesY + context->spec_data.specWorkGroupSizeY - 1) / context->spec_data.specWorkGroupSizeY, 1U);
	uint32_t workGroupCountZ = std::max((context->numParticlesZ + context->spec_data.specWorkGroupSizeZ - 1) / context->spec_data.specWorkGroupSizeZ, 1U);

	// check if size of local work group is suitable
	if (context->spec_data.specWorkGroupSizeX * context->spec_data.specWorkGroupSizeY * context->spec_data.specWorkGroupSizeZ >
				context->compute_limits.maxComputeWorkGroupInvocations) return false;

	if (workGroupCountX > context->compute_limits.maxComputeWorkGroupCount[0] || 
		workGroupCountY > context->compute_limits.maxComputeWorkGroupCount[1] || 
		workGroupCountZ > context->compute_limits.maxComputeWorkGroupCount[2] ) {

		return false;

	}

	return true;
}

void ImGuiRenderStage::addGVar(GVar* gv)
{
	switch (gv->type)
	{
	case GVAR_EVENT:
		gv->val.v_bool = ImGui::Button(gv->name.c_str());
		break;
	case GVAR_BOOL:
		ImGui::Checkbox(gv->name.c_str(), &gv->val.v_bool);
		break;
	case GVAR_FLOAT:
		ImGui::InputScalar(gv->name.c_str(), ImGuiDataType_Float, &gv->val.v_float);
		break;
	case GVAR_UINT:
		ImGui::InputScalar(gv->name.c_str(), ImGuiDataType_U32, &gv->val.v_float);
		break;
	case GVAR_VEC3:
		ImGui::InputFloat3(gv->name.c_str(), gv->val.v_vec3);
		break;
	case GVAR_DISPLAY_VALUE:
		ImGui::Text(gv->name.c_str(), gv->val.v_float);
		break;
	default:
		break;
	}
}

void ImGuiRenderStage::addGVars(GVar_Cat category)
{
	for (uint32_t i = 0; i < gVars.size(); i++)
	{
		if (gVars[i]->cat == category)
		{
			addGVar(gVars[i]);
		}
	}
}

void ImGuiRenderStage::render_gui()
{
	const ImU16 u16_one = 1;
	// Start the Dear ImGui frame
	ImGui_ImplGlfw_NewFrame();
	
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();
	//ImGui::PushFont(roboto_medium);
	// render your GUI
	ImGui::Begin("GUI v2.0");

	if (ImGui::CollapsingHeader("Hot shader reload")) {

		if (ImGui::Button("All shader")) {

			context->computeShaderChanged = true;

		}

	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Graphic Settings")) {
		addGVars(GVAR_RENDER);
	}

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Physics Settings")) {
		addGVars(GVAR_PHYSICS);
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Particle Settings")) {
		
		const char* vector_field_helper_marker_text = "Strategy determine how our particle will move accordingly.";

		enum VectorFieldMovementStrategy {
			Element_Rotate,
			Element_Translation,
			Element_Explosion,
			Element_Nothing,
			Element_COUNT
		};


		if (ImGui::TreeNode("Vector field specifics")) {
			ImGui::SameLine();
			HelpMarker(vector_field_helper_marker_text);

			static int strategies[NUM_VECTOR_FIELDS] = {	VectorFieldMovementStrategy::Element_Rotate,
															VectorFieldMovementStrategy::Element_Rotate,
															VectorFieldMovementStrategy::Element_Rotate };

			static float translation_directions[NUM_VECTOR_FIELDS][3] =

														{	
															{	context->vectorFieldTranslationDirections[0].x,
																context->vectorFieldTranslationDirections[0].y,
																context->vectorFieldTranslationDirections[0].z
															},

															{	context->vectorFieldTranslationDirections[1].x,
																context->vectorFieldTranslationDirections[1].y,
																context->vectorFieldTranslationDirections[1].z
															},

															{	context->vectorFieldTranslationDirections[2].x,
																context->vectorFieldTranslationDirections[2].y,
																context->vectorFieldTranslationDirections[2].z
															}
														};

			static int rotation_axis_selection[NUM_VECTOR_FIELDS] = { 0,1,2 };

			static float velocityStrength[NUM_VECTOR_FIELDS] = {context->vectorFieldVelocityStrength[0], 
																context->vectorFieldVelocityStrength[1], 
																context->vectorFieldVelocityStrength[2]};
			
			for (int i = 0; i < NUM_VECTOR_FIELDS; i++) {

				std::string field_label = "Vector field " + std::to_string(i) + " specifics";

				if (ImGui::TreeNode(field_label.c_str())) {

					const char* strategy_names[VectorFieldMovementStrategy::Element_COUNT] = 
																				{ "Rotation", "Translation", "White Noise", "No Movement" };
					const char* strategy_name = (	strategies[i] >= 0 &&
													strategies[i] < VectorFieldMovementStrategy::Element_COUNT) ?
													strategy_names[strategies[i]] : "Unknown";

					ImGui::SliderInt("Choose strategy", &strategies[i], 0, VectorFieldMovementStrategy::Element_COUNT - 1,
																						strategy_name);
			

					if (VectorFieldMovementStrategy(strategies[i]) == VectorFieldMovementStrategy::Element_Translation) {

						if (ImGui::TreeNode("Customize the vector field translation")) {

							ImGui::InputFloat3("Change translation direction", translation_directions[i]);

							context->vectorFieldTranslationDirections[i] = glm::vec3(	translation_directions[i][0],
																						translation_directions[i][1],
																						translation_directions[i][2]);
							ImGui::TreePop();
						}

					}
					else if (VectorFieldMovementStrategy(strategies[i]) == VectorFieldMovementStrategy::Element_Rotate) {

						if (ImGui::TreeNode("Customize the particle rotation")) {

							ImGui::RadioButton("Rotate X-Axis", &rotation_axis_selection[i], 0); ImGui::SameLine();
							ImGui::RadioButton("Rotate Y-Axis", &rotation_axis_selection[i], 1); ImGui::SameLine();
							ImGui::RadioButton("Rotate Z-Axis", &rotation_axis_selection[i], 2);

							ImGui::TreePop();

						}

					}

					ImGui::InputFloat("Strength", &velocityStrength[i], 0.0F, 0.0F, "%.6f");
					/*if (ImGui::Button("Applay changes")) {
						context->vectorFieldVelocityStrength[i] = velocityStrength[i];
						context->vectorFieldChanged = true;
					}*/
				
					ImGui::TreePop();

				}
			}

			ImGui::Text("Properties all vector fields share:");
			// vars for handling user input 
			static int vectorFieldDimX		= context->vector_field_dim.x;
			static int vectorFieldDimY		= context->vector_field_dim.y;
			static int vectorFieldDimZ		= context->vector_field_dim.z;


			ImGui::InputScalar("New vector field dim X", ImGuiDataType_U16, &vectorFieldDimX, &u16_one, NULL, "%u");
			ImGui::InputScalar("New vector field dim Y", ImGuiDataType_U16, &vectorFieldDimY, &u16_one, NULL, "%u");
			ImGui::InputScalar("New vector field dim Z", ImGuiDataType_U16, &vectorFieldDimZ, &u16_one, NULL, "%u");

			if (ImGui::Button("Recreate vector field")) {

				if (context->vector_field_dim.is_valid()) {

					context->vectorFieldChanged				= true;
					context->vector_field_dim.x				= vectorFieldDimX;
					context->vector_field_dim.y				= vectorFieldDimY;
					context->vector_field_dim.z				= vectorFieldDimZ;

					for (int i = 0; i < 3; i++) {

						context->vectorFieldVelocityStrength[i] = velocityStrength[i];

					}

					for (int i = 0; i < NUM_VECTOR_FIELDS; i++) {

						if (context->vectorFieldStrategies[i] != nullptr) {
							delete context->vectorFieldStrategies[i];
							context->vectorFieldStrategies[i] = 0;
						}

						switch (VectorFieldMovementStrategy(strategies[i])) {
							case VectorFieldMovementStrategy::Element_Rotate :
								context->vectorFieldStrategies[i] = new RotationMovement(rotation_axis_selection[i]);
								break;
							case VectorFieldMovementStrategy::Element_Translation:
								context->vectorFieldStrategies[i] = new TranslationMovement(context->vectorFieldTranslationDirections[i]);
								break;
							case VectorFieldMovementStrategy::Element_Explosion:
								context->vectorFieldStrategies[i] = new Explosion;
								break;
							case VectorFieldMovementStrategy::Element_Nothing:
								context->vectorFieldStrategies[i] = new NoMovement;
								break;
							default:
								context->vectorFieldStrategies[i] = new RotationMovement(rotation_axis_selection[i]);
							

						}

					}
		
				}

			}

			ImGui::TreePop();
		}
		else {
			ImGui::SameLine();
			HelpMarker(vector_field_helper_marker_text);
		}

		ImGui::Separator();

		const char* particles_helper_marker_text = "Choose the number of particles.";

		if (ImGui::TreeNode("Particles")) {
			ImGui::SameLine();
			HelpMarker(particles_helper_marker_text);

			static int particles_type = context->particleType;

			const char* particle_type_names[ParticleTypes::Element_COUNT] =
												{ "Glue", "Bounce"};

			const char* particle_type_name = (	particles_type >= 0 &&
												particles_type < ParticleTypes::Element_COUNT) ?
												particle_type_names[particles_type] : "Unknown";

			ImGui::SliderInt("Choose strategy", &particles_type, 0, ParticleTypes::Element_COUNT - 1,
																particle_type_name);

			ImGui::Text("# Particles X: %i",		context->numParticlesX);
			ImGui::Text("# Particles Y: %i",		context->numParticlesY);
			ImGui::Text("# Particles Z: %i",		context->numParticlesZ);
			ImGui::Text("# Particles ALL: %i",		context->numParticlesX * 
													context->numParticlesY *
													context->numParticlesZ);

			ImGui::Separator();

			ImGui::Text("Area of influence in X: %f", context->particleAreaOfInfluenceX);
			ImGui::Text("Area of influence in Y: %f", context->particleAreaOfInfluenceY);
			ImGui::Text("Area of influence in Z: %f", context->particleAreaOfInfluenceZ);

			// vars for handling user input 
			static float particleAreaOfInfluenceX = context->particleAreaOfInfluenceX;
			static float particleAreaOfInfluenceY = context->particleAreaOfInfluenceY;
			static float particleAreaOfInfluenceZ = context->particleAreaOfInfluenceZ;

			static int numberParticlesX = context->numParticlesX;
			static int numberParticlesY = context->numParticlesY;
			static int numberParticlesZ = context->numParticlesZ;

			static float particle_translation[3] = {	context->particleTranslation.x,
														context->particleTranslation.y,
														context->particleTranslation.z };

			static float particleVelocity = context->particleVelocity;

			ImGui::InputScalar("New # particles X", ImGuiDataType_U16, &numberParticlesX, &u16_one, NULL, "%u");
			ImGui::InputScalar("New # particles Y", ImGuiDataType_U16, &numberParticlesY, &u16_one, NULL, "%u");
			ImGui::InputScalar("New # particles Z", ImGuiDataType_U16, &numberParticlesZ, &u16_one, NULL, "%u");

			ImGui::Separator();
			ImGui::InputFloat("Particle scale in X", &particleAreaOfInfluenceX);
			ImGui::InputFloat("Particle scale in Y", &particleAreaOfInfluenceY);
			ImGui::InputFloat("Particle scale in Z", &particleAreaOfInfluenceZ);

			ImGui::InputFloat3("Change particle translation", particle_translation);
			//ImGui::InputScalar("New area of influence in X", ImGuiDataType_U16, &particleAreaOfInfluenceX, &u16_one, NULL, "%u");
			//ImGui::InputScalar("New area of influence in Y", ImGuiDataType_U16, &particleAreaOfInfluenceY, &u16_one, NULL, "%u");
			//ImGui::InputScalar("New area of influence in Z", ImGuiDataType_U16, &particleAreaOfInfluenceZ, &u16_one, NULL, "%u");

			ImGui::Text("Uniforms:");
			ImGui::InputFloat("Particle velocity", &particleVelocity, 0.0F, 0.0F, "%.6f");

			if (ImGui::Button("Update particle info")) {

				context->particleVelocity = particleVelocity;
				glm::mat4 new_particle_model = glm::mat4(1.f);
				new_particle_model = glm::scale(new_particle_model, glm::vec3(	particleAreaOfInfluenceX,
																				particleAreaOfInfluenceY,
																				particleAreaOfInfluenceZ));

				context->particleModel = glm::translate(new_particle_model, glm::vec3(	particle_translation[0],
																						particle_translation[1],
																						particle_translation[2]));

				context->computeUniformChanged = true;

				context->particleTranslation = glm::vec3(particle_translation[0],
														particle_translation[1],
														particle_translation[2]);

				context->particleAreaOfInfluenceX	= particleAreaOfInfluenceX;
				context->particleAreaOfInfluenceY = particleAreaOfInfluenceY;
				context->particleAreaOfInfluenceZ = particleAreaOfInfluenceZ;

				context->particleType = ParticleTypes(particles_type);


			}

			if (ImGui::Button("Apply changes")) {

				if (newNumParticlesAreSuitable(numberParticlesX, numberParticlesY, numberParticlesZ)) {

					context->computeUniformChanged = true;
					glm::mat4 new_particle_model	= glm::mat4(1.f);
					new_particle_model				= glm::scale(new_particle_model, glm::vec3(	particleAreaOfInfluenceX,
																								particleAreaOfInfluenceY,
																								particleAreaOfInfluenceZ));

					context->particleModel			= glm::translate(new_particle_model, glm::vec3(	particle_translation[0],
																									particle_translation[1],
																									particle_translation[2]));

					context->numParticlesChanged = true;

					context->numParticlesX = numberParticlesX;
					context->numParticlesY = numberParticlesY;
					context->numParticlesZ = numberParticlesZ;

					context->particleAreaOfInfluenceX = particleAreaOfInfluenceX;
					context->particleAreaOfInfluenceY = particleAreaOfInfluenceY;
					context->particleAreaOfInfluenceZ = particleAreaOfInfluenceZ;

					context->particleVelocity = particleVelocity;

					context->particleType = ParticleTypes(particles_type);


				}
				else {
					ImGui::OpenPopup("# particles not compatible with workgroup size!");
				}

			}

			if (ImGui::BeginPopupModal("# particles not compatible with workgroup size!", NULL))
			{

				ImGui::Text("The work group sizes are not possible on your device.");

				ImGui::Text("Check your device limitations. You might have exceeded them.");

				int numWorkGroupX = (numberParticlesX + context->spec_data.specWorkGroupSizeX - 1) / context->spec_data.specWorkGroupSizeX;
				int numWorkGroupY = (numberParticlesY + context->spec_data.specWorkGroupSizeY - 1) / context->spec_data.specWorkGroupSizeY;
				int numWorkGroupZ = (numberParticlesZ + context->spec_data.specWorkGroupSizeZ - 1) / context->spec_data.specWorkGroupSizeZ;

				ImGui::Text("Your values would result in #%i invocations, but only #%i are allowed.",
																			numWorkGroupX *
																			numWorkGroupY *
																			numWorkGroupZ,
																			context->compute_limits.maxComputeWorkGroupInvocations);

				ImGui::Text("Calculation rules of #work groups are as follows:");
				ImGui::Text("numWorkGroup(-X/-Y/-Z) = numParticles(-X/-Y/-Z) + workGroupSize(-X/-Y/-Z) - 1) / workGroupSize(-X/-Y/-Z)");

				if (ImGui::Button("Close"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			ImGui::TreePop();

		}
		else {

			ImGui::SameLine();
			HelpMarker(particles_helper_marker_text);

		}

		ImGui::Separator();

		const char* workgroup_helper_marker_text =	"Have a look on your device limits for the workgroups and adjust" 
													"your workgroup size as you want.";

		if (ImGui::TreeNode("Workgroup")) {
			ImGui::SameLine();
			HelpMarker(workgroup_helper_marker_text);
			if (ImGui::BeginTable("Workgroup", 2))
			{

				// set header row
				ImGui::TableSetupColumn("Current value");
				ImGui::TableSetupColumn("Device limit");
				ImGui::TableHeadersRow();

				int row = 0;
				int column = 0;
				// workgroupsizes
				column = 0;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(column);
				ImGui::Text("Workgroup size X: %i, ", context->spec_data.specWorkGroupSizeX);
				column++;

				ImGui::TableSetColumnIndex(column);
				ImGui::Text("%.i", context->compute_limits.maxComputeWorkGroupSize[0]);
				column++;

				row++;

				column = 0;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(column);
				ImGui::Text("Workgroup size Y: %i, ", context->spec_data.specWorkGroupSizeY);
				column++;

				ImGui::TableSetColumnIndex(column);
				ImGui::Text("%.i", context->compute_limits.maxComputeWorkGroupSize[1]);
				column++;

				row++;

				column = 0;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(column);
				ImGui::Text("Workgroup size Z: %i, ", context->spec_data.specWorkGroupSizeZ);
				column++;

				ImGui::TableSetColumnIndex(column);
				ImGui::Text("%.i", context->compute_limits.maxComputeWorkGroupSize[2]);
				column++;

				row++;
				// max workgroup invocations 
				column = 0;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(column);
				ImGui::Text("Workgroup invocat: %i, ", ((context->numParticlesX + context->spec_data.specWorkGroupSizeX - 1) / context->spec_data.specWorkGroupSizeX) *
					((context->numParticlesY + context->spec_data.specWorkGroupSizeY - 1) / context->spec_data.specWorkGroupSizeY) *
					((context->numParticlesZ + context->spec_data.specWorkGroupSizeZ - 1) / context->spec_data.specWorkGroupSizeZ));
				column++;

				ImGui::TableSetColumnIndex(column);
				ImGui::Text("%.i", context->compute_limits.maxComputeWorkGroupInvocations);
				column++;

				row++;


				// workgroup count 
				column = 0;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(column);
				ImGui::Text("Workgroup count X: %i, ", (context->numParticlesX + context->spec_data.specWorkGroupSizeX - 1) / context->spec_data.specWorkGroupSizeX);
				column++;

				ImGui::TableSetColumnIndex(column);
				ImGui::Text("%.i", context->compute_limits.maxComputeWorkGroupCount[0]);
				column++;

				row++;

				column = 0;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(column);
				ImGui::Text("Workgroup count Y: %i, ", (context->numParticlesY + context->spec_data.specWorkGroupSizeY - 1) / context->spec_data.specWorkGroupSizeY);
				column++;

				ImGui::TableSetColumnIndex(column);
				ImGui::Text("%.i", context->compute_limits.maxComputeWorkGroupCount[1]);
				column++;

				row++;

				column = 0;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(column);
				ImGui::Text("Workgroup count Z: %i, ", (context->numParticlesZ + context->spec_data.specWorkGroupSizeZ - 1) / context->spec_data.specWorkGroupSizeZ);
				column++;

				ImGui::TableSetColumnIndex(column);
				ImGui::Text("%.i", context->compute_limits.maxComputeWorkGroupCount[2]);
				column++;

				row++;

				ImGui::EndTable();

				ImGui::Separator();

				const char* work_group_test_marker_text =	"Automatically chooses best work group size for chosen setup "
															"and apply them :)";
				if (ImGui::TreeNode("Work group size testing")) {
					ImGui::SameLine(); HelpMarker(work_group_test_marker_text);

					if(ImGui::Button("Run test")) {

						context->workGroupTesting = true;

					}

					ImGui::ProgressBar(context->workGroupTestingProgess, ImVec2(0.0f,0.0f));

					ImGui::Text("Best workgroup size combo (X,Y,Z): %i, %i, %i", 
																context->bestWorkGroupSizeX, 
																context->bestWorkGroupSizeY,
																context->bestWorkGroupSizeZ);

					ImGui::TreePop();
				}
				else {
					ImGui::SameLine(); HelpMarker(work_group_test_marker_text);
				}

			}

			ImGui::Separator();

			static int workGroupSizeX = context->spec_data.specWorkGroupSizeX;
			static int workGroupSizeY = context->spec_data.specWorkGroupSizeY;
			static int workGroupSizeZ = context->spec_data.specWorkGroupSizeZ;

			ImGui::InputScalar("New work group size X", ImGuiDataType_U16, &workGroupSizeX, &u16_one, NULL, "%u");
			ImGui::InputScalar("New work group size Y", ImGuiDataType_U16, &workGroupSizeY, &u16_one, NULL, "%u");
			ImGui::InputScalar("New work group size Z", ImGuiDataType_U16, &workGroupSizeZ, &u16_one, NULL, "%u");

			if (ImGui::Button("Apply changes")) {
				if (newWorkgroupSizesAreSuitable(workGroupSizeX, workGroupSizeY, workGroupSizeZ)) {

					context->spec_data.specWorkGroupSizeX = workGroupSizeX;
					context->spec_data.specWorkGroupSizeY = workGroupSizeY;
					context->spec_data.specWorkGroupSizeZ = workGroupSizeZ;

					// recreate as in hot shader reload 
					context->computeShaderChanged = true;

				}
				else {

					ImGui::OpenPopup("No good workgroupsizes!");
					
				}
			}

			if (ImGui::BeginPopupModal("No good workgroupsizes!", NULL))
			{
				
				ImGui::Text("The work group sizes are not possible on your device.");

				ImGui::Text("Check your device limitations. You might have exceeded them.");

				int numWorkGroupX = (context->numParticlesX + workGroupSizeX - 1) / workGroupSizeX;
				int numWorkGroupY = (context->numParticlesY + workGroupSizeY - 1) / workGroupSizeY;
				int numWorkGroupZ = (context->numParticlesZ + workGroupSizeZ - 1) / workGroupSizeZ;

				ImGui::Text("Your values would result in #%i invocations, but only #%i are allowed.",	
																					numWorkGroupX * 
																					numWorkGroupY * 
																					numWorkGroupZ,
																					context->compute_limits.maxComputeWorkGroupInvocations);

				ImGui::Text("Calculation rules of #work groups are as follows:");
				ImGui::Text("numWorkGroup(-X/-Y/-Z) = numParticles(-X/-Y/-Z) + workGroupSize(-X/-Y/-Z) - 1) / workGroupSize(-X/-Y/-Z)");

				if (ImGui::Button("Close"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			ImGui::TreePop();
		
		}
		else {
			
			ImGui::SameLine();
			HelpMarker(workgroup_helper_marker_text);

		}
		

		ImGui::Separator();

		const char* timings_helper_marker_text =
			"Timings of all our individual compute stages. The timing of the whole compute pass includes all stages + copy buffer/... etc. commands.";

		if (ImGui::TreeNode("Compute timings")) {
			ImGui::SameLine();
			HelpMarker(timings_helper_marker_text);
			//ImGui::Text("Stages:");
			ImGui::Text("Simulation (ms) %.3f", context->time_simulation_stage_ms);
			ImGui::Text("Integration (ms) %.3f", context->time_integration_stage_ms);
			//ImGui::Text("Complete pass:");
			ImGui::Text("Whole compute pass (ms) %.3f", context->time_compute_pass_ms);
			ImGui::TreePop();

		}
		else {
			ImGui::SameLine();
			HelpMarker(timings_helper_marker_text);
		}

	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("GUI Settings")) {

		ImGuiStyle& style = ImGui::GetStyle();

		if (ImGui::SliderFloat("Frame Rounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f")) {
			style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
		}
		{ bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder", &border)) { style.FrameBorderSize = border ? 1.0f : 0.0f; } }
		ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("KEY Bindings")) {

		ImGui::Text("WASD for moving Forward, backward and to the side\n QE for rotating ");

	}

	ImGui::Separator();

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();

}

void ImGuiRenderStage::recordRenderCommands(VkCommandBuffer& cmdBuf, DrawParams params)
{
	vkCmdNextSubpass(cmdBuf, VK_SUBPASS_CONTENTS_INLINE);
	render_gui();
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
}

Subpass* ImGuiRenderStage::getSubpass()
{
    return subpass;
}