#include "Model.h"


Model::Model()
{
}

Model::Model(std::vector<Vertex> vertices, std::vector<Index> indices)
{
	this->vertices	= vertices;
	this->indices	= indices;
}

Model::Model(std::vector<Vertex> vertices, std::vector<Index> indices, std::vector<Surface>& surfaces, std::vector<PointLight> pointLights, glm::vec3 centerOffMass)
{
	this->vertices = vertices;
	this->indices = indices;
	this->surfaces = surfaces;
	this->pointLights = pointLights;
	this->centerOfMass = centerOffMass;
}
Model::Model(std::vector<Vertex> vertices, std::vector<Index> indices, std::vector<PointLight> pointLights)
{
	this->vertices		= vertices;
	this->indices		= indices;
	this->pointLights	= pointLights;
}

Model::~Model()
{
}



Model Model::getDefaultCube()
{
	std::vector<Vertex> cubeVertecies =
	{
		Vertex(glm::vec3(-1.0, -1.0,  1.0)),
		Vertex(glm::vec3(1.0, -1.0,  1.0) ),
		Vertex(glm::vec3(1.0,  1.0,  1.0) ),
		Vertex(glm::vec3(-1.0,  1.0,  1.0)),
		Vertex(glm::vec3(-1.0, -1.0, -1.0)),
		Vertex(glm::vec3(1.0, -1.0, -1.0) ),
		Vertex(glm::vec3(1.0,  1.0, -1.0) ),
		Vertex(glm::vec3(-1.0,  1.0, -1.0))
	};
	std::vector<Index> cubeIndices =
	{
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3
	};
	return Model(cubeVertecies, cubeIndices);
}
