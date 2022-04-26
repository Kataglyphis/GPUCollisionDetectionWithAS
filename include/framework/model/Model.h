#pragma once
#include "Vertex.h"
#include "ShaderStructs.h"

typedef uint32_t Index;


struct Surface
{
	uint32_t startIndex;
	uint32_t numIndices;
	Material mat;
	uint32_t matIndex;
};

class Model
{
public:
	Model();
	Model(std::vector<Vertex> vertices, std::vector<Index> indices);
	Model(std::vector<Vertex> vertices, std::vector<Index> indices, std::vector<Surface>& surfaces, std::vector<PointLight> pointLights, glm::vec3 centerOffMass);
	Model(std::vector<Vertex> vertices, std::vector<Index> indices, std::vector<PointLight> pointLights);
	~Model();


	static Model getDefaultCube();


	std::vector<Vertex>		vertices;
	std::vector<Index>		indices;
	std::vector<Surface>	surfaces;


	std::vector<uint32_t>	materialIndices;

	glm::vec3				centerOfMass;

	// A, model can have point lights attached to it
	std::vector<PointLight> pointLights;

private:


};

