#include "Particles.h"

Particles::Particles(	uint32_t numParticlesX, 
						uint32_t numParticlesY, 
						uint32_t numParticlesZ,
						glm::mat4 particleModel)

						{
	amountParticles.x_dim = numParticlesX;
	amountParticles.y_dim = numParticlesY;
	amountParticles.z_dim = numParticlesZ;

	this->particleModel = particleModel;

	particle_data.resize(numParticlesX * numParticlesY * numParticlesZ);

	create();

}

Particles::Particles()
{
}

std::vector<Particle> Particles::get_data()
{
	return particle_data;
}

Particles::~Particles()
{
}

void Particles::create()
{

	// the number of levels our texture has in the z-dimension
	/*std::string base_dir = "../Resources/Textures/Particles/";
	std::string file_name_color = base_dir + "WhiteNoise/WhiteNoise_" + std::to_string(0) + ".png";
	stbi_set_flip_vertically_on_load(true);
	VkDeviceSize size;
	int width;
	int height;
	unsigned short* image_color = load_texture_file(file_name_color, &width, &height, &size);*/

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<> dist(0.0, 1.0);

#pragma omp parallel
	for (int z = 0; z < amountParticles.z_dim; z++) {

		//std::string file_name_position = base_dir + "Position/Position_" + std::to_string(i) + ".png";
		//std::string file_name_velocity = base_dir + "VectorFields/Rotation/Rotation_" + std::to_string(i) + ".png";

		// stb_image.h reads images from upper left corner; therefore flip y-coordinate accordingly

		//unsigned short* image_position = load_texture_file(file_name_position, &width, &height, &size);
		//unsigned short* image_velocity = load_texture_file(file_name_velocity, &width, &height, &size);

		// Pay attention: 
		// --------- 1.) each pixel has 4 channels (4 unsigned chars) therefore one has to multiply the offset by number of channels 
		// --------- 2.) we want the values need normalization step (--> we want values in [0,1])
		// --------- 3.) one has to cast the unsigned chars to unsigned int for we want numbers in our storage buffer
		// ---------	and not chars
		int numberOfChannels = 4;

		for (unsigned int y = 0; y < amountParticles.y_dim; y++) {

			for (unsigned int x = 0; x < amountParticles.x_dim; x++) {

				// ------ read values in; ignore alpha channel
				glm::vec3 pos = {dist(rng), dist(rng), dist(rng)};/*{	(float)x / (float)amountParticles.x_dim,
									(float)y / (float)amountParticles.y_dim,
									(float)z / (float)amountParticles.z_dim };*/

				glm::vec3 color = { 1.f, 1.f, 1.f/*static_cast<unsigned int>(image_color[(y * amountParticles.x_dim + x) * numberOfChannels]),
									static_cast<unsigned int>(image_color[(y * amountParticles.y_dim + x) * numberOfChannels + 1]),
									static_cast<unsigned int>(image_color[(y * amountParticles.z_dim + x) * numberOfChannels + 2])*/ };


				glm::vec3 vel = { 0.0,0.0,0.0 };//  { dist(rng), dist(rng), dist(rng) };
									/*{ static_cast<unsigned int>(image_velocity[(j * width + m) * numberOfChannels]),
									static_cast<unsigned int>(image_velocity[(j * width + m) * numberOfChannels + 1]),
									static_cast<unsigned int>(image_velocity[(j * width + m) * numberOfChannels + 2])};*/
				glm::vec3 accel = { 0.0,0.0,0.0 };
				// ----- normalization step
				// ----- 
				//pos /= sizeof(unsigned char) * std::pow(2, 16);
				//color /= sizeof(unsigned char) * std::pow(2, 16);
				//vel /= sizeof(unsigned char) * std::pow(2, 16);

				// bring the values from position and velocity from [0,1] to [-1,1]
				pos *= 2.f;
				pos -= 1.f;
				//vel *= 2.f;
				//vel -= 1.f;

				// ----- transform particle position
				// pos = glm::vec3(particleModel * glm::vec4(pos, 1.0f));

				// ----- index for our storage buffer 
				int index = (z * amountParticles.x_dim * amountParticles.y_dim) + (y * amountParticles.x_dim + x);

				particle_data[index] = { glm::vec4(pos,0.0), glm::vec4(color,1.0), glm::vec4(vel,0.0f), glm::vec4(accel,0.0f) };

			}

		}

	}

	//writeToFile();

}

void Particles::writeToFile()
{

	std::vector<uint32_t> dims = { amountParticles.x_dim, amountParticles.y_dim, amountParticles.z_dim };
	std::string dir_name = std::string("../Resources/Data/");
	std::string file_name = std::string("particles.dat");
	std::string full_file_path = dir_name + file_name;

	file_writer.writeParticlesToFile(full_file_path, "4 Lines per Particle: position, velocity, acceleration and color ", dims, particle_data);

}

stbi_us* Particles::load_texture_file(std::string file_name, int* width, int* height, VkDeviceSize* image_size)
{
	// number of channels image uses
	int channels;
	// load pixel data for image
	std::string file_loc = file_name;
	stbi_us* image = stbi_load_16(file_loc.c_str(), width, height, &channels, STBI_rgb_alpha);

	if (!image) {

		throw std::runtime_error("Failed to load a texture file! (" + file_name + ")");

	}

	// calculate image size using given and known data
	*image_size = *width * *height * 4;

	return image;
}
