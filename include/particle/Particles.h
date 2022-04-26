#pragma once
#include <cstdint>
#include <Common.h>
#include <string>
#include "stb_image.h"
#include <random>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <FileWriter.h>

class Particles
{
public: 

	Particles(	uint32_t numParticlesX,
				uint32_t numParticlesY, 
				uint32_t numParticlesZ,
				glm::mat4 particleModel);

	Particles();

	std::vector<Particle> get_data();

	~Particles();

private:

	void create();
	void writeToFile();

	stbi_us* load_texture_file(std::string file_name, int* width, int* height, VkDeviceSize* image_size);

	std::vector<Particle> particle_data;

	FileWriter file_writer{};

	struct AmountOfParticles {

		uint32_t x_dim;
		uint32_t y_dim;
		uint32_t z_dim;

	} amountParticles;

	float particleAreaOfInfluenceX;
	float particleAreaOfInfluenceY;
	float particleAreaOfInfluenceZ;

	glm::mat4 particleModel;

};

