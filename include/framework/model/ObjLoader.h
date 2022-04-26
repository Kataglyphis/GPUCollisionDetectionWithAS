#pragma once
#include "Model.h"
#include "MaterialManager.h"


#include <tiny_obj_loader.h>

class ObjLoader
{
public:
	ObjLoader(MaterialManager* materialManager);
	~ObjLoader();

	Model* load(const char* path, const char* mtlBaseDir, const char* textureBaseDir);

	void loadPointLight(tinyobj::mesh_t& mesh, tinyobj::attrib_t& vertexAttributes, std::string name, std::vector<PointLight>& pointLights);

	void loadMesh(tinyobj::mesh_t& mesh, tinyobj::attrib_t& vertexAttributes, std::vector<Vertex>& vertices, std::vector<Index>& indices, std::vector<Surface>& surfaces, std::unordered_map<Vertex, uint32_t>& vertexMap);

	Vertex loadVertex(tinyobj::attrib_t& vertexAttributes, uint32_t materialIndex, tinyobj::index_t index);

	void loadFace(tinyobj::mesh_t& mesh, tinyobj::attrib_t& vertexAttributes, std::unordered_map<Vertex, uint32_t>& vertexMap, std::vector<Vertex>& vertices, std::vector<Index>& indices, std::vector<Surface>& surfaces, uint32_t faceIndex, uint32_t& index_offset);

	void calculateTangentAndBitangent(std::vector<Vertex>& vertices, std::vector<Index>& indices);

private:
	MaterialManager*	materialManager;
	uint32_t			materialIndexOffset;
};


