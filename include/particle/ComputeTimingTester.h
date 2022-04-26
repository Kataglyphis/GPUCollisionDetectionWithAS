#pragma once
#include <CSVWriter.h>
#include <Common.h>
#include <Context.h>
#include <iostream>
#include <string>
#include <algorithm>

class ComputeTimingTester
{

public:

	ComputeTimingTester();

	void initialize(Context* context);

	void next();

	~ComputeTimingTester();

private:

	CSVWriter fileWriter;

	const uint32_t numIterationsPerTest = 10;

	uint32_t current_iteration;
	uint32_t current_iteration_total;
	int32_t numWorkGroupCombinations;
	uint32_t numStepsTotal;

	// build sum over all times so we can 
	float time_simulation_pass;
	float time_integration_pass;
	float time_compute_pass;

	Context* context;

	std::vector<uint32_t> possible_workgroup_sizes_x;
	std::vector<uint32_t> possible_workgroup_sizes_y;
	std::vector<uint32_t> possible_workgroup_sizes_z;

	float lowest_time_compute_pass;

	bool check_work_group_size_candidates(int workgroup_size_candidate_x, int workgroup_size_candidate_y, int workgroup_size_candidate_z);

};

