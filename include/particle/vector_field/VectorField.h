#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <FileWriter.h>
#include <MovementStrategy.h>
#include <RotationMovement.h>
#include <TranslationMovement.h>
#include <Explosion.h>

class VectorField
{
public:

	VectorField();
	VectorField(uint32_t width, uint32_t height, uint32_t depth, MovementStrategy* movement_strategy, glm::mat4 vector_field_model);

	void					create();
	std::vector<glm::vec4>	get_velocity_data();

	~VectorField();

private:

	FileWriter file_writer{};

	MovementStrategy* movement_strategy;

	std::vector<glm::vec4> velocity_data;

	VectorFieldDimensions dimensions;

	void writeToFile();

	glm::vec3 position;
	glm::mat4 vector_field_model;

};

