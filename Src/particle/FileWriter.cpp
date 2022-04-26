#include "FileWriter.h"

FileWriter::FileWriter()
{
}

void FileWriter::write4dDataToFile(std::string file_name, std::string data_layout, std::vector<uint32_t> dimensions, std::vector<glm::vec4> data)
{
	// Create and open a text file
	std::ofstream MyFile(file_name);

	// Write layout in first row
	MyFile << data_layout << "\n";

	// write vector field dimensions to one row 
	MyFile << dimensions[0] << " " << dimensions[1] << " " << dimensions[2] << "\n";

	// now write data to file with this layout 
		for (unsigned int z = 0; z < dimensions[2]; z++) {
			for (unsigned int y = 0; y < dimensions[1]; y++) {
				for (unsigned int x = 0; x < dimensions[0]; x++) {

				MyFile <<	data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].x << " " <<
							data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].y << " " <<
							data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].z <<
							"\n";

			}
		}

	}

	// Close the file
	MyFile.close();
}

void FileWriter::writeParticlesToFile(std::string file_name, std::string data_layout, std::vector<uint32_t> dimensions, std::vector<Particle> data)
{

	// Create and open a text file
	std::ofstream MyFile(file_name);

	// Write layout in first row
	MyFile << data_layout << "\n";

	// write vector field dimensions to one row 
	MyFile << dimensions[0] << " " << dimensions[1] << " " << dimensions[2] << "\n";

	// now write data to file with this layout 
	for (unsigned int z = 0; z < dimensions[2]; z++) {
		for (unsigned int y = 0; y < dimensions[1]; y++) {
			for (unsigned int x = 0; x < dimensions[0]; x++) {
				MyFile << 
					// write particle position to line 
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].position.x << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].position.y << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].position.z << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].position.w << " " <<
					"\n" <<
					// write start velocity to file
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].velocity.x << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].velocity.y << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].velocity.z << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].velocity.w << " " <<
					"\n" <<
					// write start acceleration to file
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].acceleration.x << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].acceleration.y << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].acceleration.z << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].acceleration.w << " " <<
					"\n" <<
					// write color to line 
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].color.x << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].color.y << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].color.z << " " <<
					data[dimensions[1] * dimensions[0] * z + y * dimensions[0] + x].color.w << " " <<
					"\n" << 
					// mark end of particle 
					"\n";
			}
		}

	}

	// Close the file
	MyFile.close();

}

FileWriter::~FileWriter()
{
}
