#include "VectorField.h"

VectorField::VectorField()
{
	dimensions.x = 10;
	dimensions.y = 10;
	dimensions.z = 10;

	position = glm::vec3(0.0f);

	velocity_data.resize(10 * 10 * 10);
	// for now we only support rotation strategy
	movement_strategy = new RotationMovement(0);

	create();
}

VectorField::VectorField(uint32_t width, uint32_t height, uint32_t depth, MovementStrategy* movement_strategy, glm::mat4 vector_field_model)
{
	dimensions.x = width;
	dimensions.y = height;
	dimensions.z = depth;

	velocity_data.resize(width * height * depth);
	// for now we only support rotation strategy

	this->vector_field_model = vector_field_model;

	this->movement_strategy = movement_strategy;

	create();


}

void VectorField::create()
{

#pragma omp parallel
	for (int z = 0; z < dimensions.z; z++) {
		for (int y = 0; y < dimensions.y; y++) {
			for (int x = 0; x < dimensions.x; x++) {

				glm::vec3 remapped_position =	glm::vec3(	x - (static_cast<float>(dimensions.x) / 2.f),
															y - (static_cast<float>(dimensions.y) / 2.f),
															z - (static_cast<float>(dimensions.z) / 2.f));

				/*glm::vec3 vector_field_model_pos = glm::vec3(vector_field_model * glm::vec4(remapped_position, 1.0f));*/

				velocity_data[	z * dimensions.x *	dimensions.y	+
								y *	dimensions.x					+ 
								x] 
								= glm::vec4(movement_strategy->get_velocity(remapped_position), 0.0f);

			}
		}
	}

	writeToFile();
}

std::vector<glm::vec4> VectorField::get_velocity_data()
{
	return velocity_data;
}

VectorField::~VectorField()
{
	
}

void VectorField::writeToFile()
{
	std::vector<uint32_t> dims = { dimensions.x, dimensions.y, dimensions.z };
	std::string dir_name = std::string("../Resources/Data/");
	std::string file_name = std::string("vektorfield.dat");
	std::string full_file_path = dir_name + file_name;

	file_writer.write4dDataToFile(full_file_path, "x y z", dims, velocity_data);
}
