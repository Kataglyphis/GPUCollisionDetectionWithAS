#include "ComputeTimingTester.h"

ComputeTimingTester::ComputeTimingTester()
{
}

void ComputeTimingTester::initialize(Context* context)
{

	this->context = context;

	time_simulation_pass		= 0.;
	time_integration_pass		= 0.;
	time_compute_pass			= 0.;

	numWorkGroupCombinations	= 0;

	current_iteration			= 1;

	current_iteration_total		= 0;

	// init vars for storing lowest working group sizes and time 
	lowest_time_compute_pass = std::numeric_limits<unsigned int>::max();

	// precompute all possible workgroup sizes 
	// number of invocations must not exceed the global limit!
	// for testing purposes we want all possibilities 
	for (int workgroup_size_candidate_x = 1; workgroup_size_candidate_x < context->compute_limits.maxComputeWorkGroupSize[0]; workgroup_size_candidate_x *= 2) {
		for (int workgroup_size_candidate_y = 1; workgroup_size_candidate_y < context->compute_limits.maxComputeWorkGroupSize[1]; workgroup_size_candidate_y *= 2) {
			for (int workgroup_size_candidate_z = 1; workgroup_size_candidate_z < context->compute_limits.maxComputeWorkGroupSize[2]; workgroup_size_candidate_z *= 2) {

				if (check_work_group_size_candidates(workgroup_size_candidate_x, workgroup_size_candidate_y, workgroup_size_candidate_z)) {

					possible_workgroup_sizes_x.push_back(workgroup_size_candidate_x);
					possible_workgroup_sizes_y.push_back(workgroup_size_candidate_y);
					possible_workgroup_sizes_z.push_back(workgroup_size_candidate_z);
					numWorkGroupCombinations++;

				}


			}
		}
	}

	fileWriter.newRow() <<	"Workgroup size X"		<< "Workgroup size Y"		<< "Workgroup size Z"	<< 
							"Num particles X"		<< "Num particles Y"		<< "Num particles Z"	<< 
							"Simulation pass (ms)"	<< "Integration pass (ms)"	<< "Compute pass (ms)"	;


	// only if more possible workGroupSizes are available
	if (numWorkGroupCombinations == 0) return;

	numStepsTotal = numWorkGroupCombinations * numIterationsPerTest;

	context->spec_data.specWorkGroupSizeX = possible_workgroup_sizes_x.back();
	context->spec_data.specWorkGroupSizeY = possible_workgroup_sizes_y.back();
	context->spec_data.specWorkGroupSizeZ = possible_workgroup_sizes_z.back();


	// delete 'em from store
	possible_workgroup_sizes_x.pop_back();
	possible_workgroup_sizes_y.pop_back();
	possible_workgroup_sizes_z.pop_back();

	context->computeShaderChanged = true;

	numWorkGroupCombinations--;

}

void ComputeTimingTester::next()
{
	// this indicates that have finished testing and we no longer write to file :) 
	if (numWorkGroupCombinations == -1) {

		context->workGroupTesting = false;

		return;
	}

	// last step; now we are ready to write everything to a csv file 
	if (numWorkGroupCombinations == 0) {

		//std::cout << fileWriter << std::endl;
		std::string dir			= "../Resources/Data/";
		std::string file_name	= "workGroupExp";
		std::string numParticles = std::to_string(context->numParticlesX * context->numParticlesY * context->numParticlesZ);
		std::string file_ending = ".csv";
		fileWriter.writeToFile(dir + file_name + file_ending);
		numWorkGroupCombinations--;
		context->workGroupTestingProgess = 0.0f;

		// apply best work group sizes
		context->spec_data.specWorkGroupSizeX = context->bestWorkGroupSizeX;
		context->spec_data.specWorkGroupSizeY = context->bestWorkGroupSizeY;
		context->spec_data.specWorkGroupSizeZ = context->bestWorkGroupSizeZ;

		context->computeShaderChanged = true;

		return;
	}

	if (current_iteration == 0) {

		// 1st: write all interessting stuff to file
		time_simulation_pass	/= static_cast<float>(numIterationsPerTest);
		time_integration_pass	/= static_cast<float>(numIterationsPerTest);
		time_compute_pass		/= static_cast<float>(numIterationsPerTest);
		
		// store best combo
		if (time_compute_pass < lowest_time_compute_pass) {

			lowest_time_compute_pass = time_compute_pass;
			context->bestWorkGroupSizeX = context->spec_data.specWorkGroupSizeX;
			context->bestWorkGroupSizeY = context->spec_data.specWorkGroupSizeY;
			context->bestWorkGroupSizeZ = context->spec_data.specWorkGroupSizeZ;

		}

		// this is hacky!
		// MS Excel needs floating point numbers with a ',' instead of a '.'
		std::string time_simulation_pass_str	= std::to_string(time_simulation_pass);
		std::replace(time_simulation_pass_str.begin(), time_simulation_pass_str.end(), '.', ',');
		std::string time_integration_pass_str	= std::to_string(time_integration_pass);
		std::replace(time_integration_pass_str.begin(), time_integration_pass_str.end(), '.', ',');
		std::string time_compute_pass_str		= std::to_string(time_compute_pass);
		std::replace(time_compute_pass_str.begin(), time_compute_pass_str.end(), '.', ',');

		fileWriter.newRow() << int(context->spec_data.specWorkGroupSizeX)	<< int(context->spec_data.specWorkGroupSizeY)	<< int(context->spec_data.specWorkGroupSizeZ)
							<< int(context->numParticlesX)					<< int(context->numParticlesY)					<< int(context->numParticlesZ)
							<< time_simulation_pass_str						<< time_integration_pass_str					<<	time_compute_pass_str;
		
		std::string str = fileWriter.toString();

		// 2nd: update all stuff 
		// get next combination of workgroup sizes 
		context->spec_data.specWorkGroupSizeX = possible_workgroup_sizes_x.back();
		context->spec_data.specWorkGroupSizeY = possible_workgroup_sizes_y.back();
		context->spec_data.specWorkGroupSizeZ = possible_workgroup_sizes_z.back();

		// delete 'em from store
		possible_workgroup_sizes_x.pop_back();
		possible_workgroup_sizes_y.pop_back();
		possible_workgroup_sizes_z.pop_back();

		context->computeShaderChanged = true;

		// we start from 0 again 
		time_simulation_pass		= 0.f;
		time_integration_pass		= 0.f;
		time_compute_pass			= 0.f;

		numWorkGroupCombinations--;

	}

	time_simulation_pass				+= context->time_simulation_stage_ms;
	time_integration_pass				+= context->time_integration_stage_ms;
	time_compute_pass					+= context->time_compute_pass_ms;

	current_iteration					+= 1;
	current_iteration					= current_iteration % numIterationsPerTest;

	current_iteration_total				+= 1;

	context->workGroupTestingProgess	= (float)current_iteration_total / (float)numStepsTotal;

}

ComputeTimingTester::~ComputeTimingTester()
{
}

bool ComputeTimingTester::check_work_group_size_candidates(int workgroup_size_candidate_x, int workgroup_size_candidate_y, int workgroup_size_candidate_z)
{
	uint32_t workGroupCountX = std::max((context->numParticlesX + workgroup_size_candidate_x - 1) / workgroup_size_candidate_x, 1U);
	uint32_t workGroupCountY = std::max((context->numParticlesY + workgroup_size_candidate_y - 1) / workgroup_size_candidate_y, 1U);
	uint32_t workGroupCountZ = std::max((context->numParticlesZ + workgroup_size_candidate_z - 1) / workgroup_size_candidate_z, 1U);

	// check if size of local work group is suitable
	if (workgroup_size_candidate_x * workgroup_size_candidate_y * workgroup_size_candidate_z >
								context->compute_limits.maxComputeWorkGroupInvocations) return false;

	if (workGroupCountX > context->compute_limits.maxComputeWorkGroupCount[0] ||
		workGroupCountY > context->compute_limits.maxComputeWorkGroupCount[1] ||
		workGroupCountZ > context->compute_limits.maxComputeWorkGroupCount[2]) {

		return false;

	}

	return true;
}
