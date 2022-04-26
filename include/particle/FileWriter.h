#pragma once 

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cinttypes>
#include <glm/glm.hpp>
#include <Common.h>

class FileWriter {

public:

	FileWriter();

	void write4dDataToFile(std::string file_name, std::string data_layout, std::vector<uint32_t> dimensions, std::vector<glm::vec4> data);
	void writeParticlesToFile(std::string file_name, std::string data_layout, std::vector<uint32_t> dimensions, std::vector<Particle> data);

	~FileWriter();

private:

};

