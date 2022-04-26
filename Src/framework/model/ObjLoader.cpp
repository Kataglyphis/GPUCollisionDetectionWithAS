#include "ObjLoader.h"
#include <iostream>
#include "Utilities.h"



ObjLoader::ObjLoader(MaterialManager* materialManager)
{
	this->materialManager = materialManager;
	materialIndexOffset = 0;
}

ObjLoader::~ObjLoader()
{
}


Model* ObjLoader::load(const char* path, const char* mtlBaseDir, const char* textureBaseDir)
{
	tinyobj::attrib_t vertexAttributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errorString;
	std::string warningString;


	std::vector<Vertex>		vertices;
	std::vector<Index>		indices;
	std::vector<Surface>	surfaces;

	std::vector<PointLight>	pointLights;


	bool success = tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &warningString, &errorString, path, mtlBaseDir);

	if (!success)
	{
		std::cout << "TINYOBJ WARNING:" << warningString << std::endl;
		throw std::runtime_error(errorString);

	}
	std::cout << "Material Count: " << materials.size() << std::endl;


	// Add materials
	std::vector<uint32_t> materialIndices;
	for (uint32_t i = 0; i < materials.size(); i++)
	{
		materialIndices.push_back(materialManager->addMaterial(materials[i], textureBaseDir));
	}

	// Get vertex data

	uint32_t meshCnt = 0;


	std::unordered_map<Vertex, uint32_t> vertexMap;
	for (size_t s = 0; s < shapes.size(); s++)
	{
		if (shapes[s].name._Starts_with("POINT_LIGHT"))
		{
			std::cout << "found point light!!" << std::endl;
			loadPointLight(shapes[s].mesh, vertexAttributes, shapes[s].name, pointLights);
		}
		else if (shapes[s].name._Starts_with("DECAL"))
		{
			// Do nothing
			// Custom decal definition contained in .obj file which i used for my bachelor thesis
		}
		else
		{
			meshCnt++;
			loadMesh(shapes[s].mesh, vertexAttributes, vertices, indices, surfaces, vertexMap);
		}
	}

	glm::vec3 centerOfMass = glm::vec3(0.0);
	float coef = 1.0 / (float)vertices.size();
	for (uint32_t i = 0; i < vertices.size(); i++)
	{
		centerOfMass += glm::vec3(vertices[i].pos) * coef;
	}

	std::cout << "Done loading!" << std::endl;

	// Calculate tangent and bitangent for normal maps
	calculateTangentAndBitangent(vertices, indices);

	materialIndexOffset = materialManager->getMaterialCount();

	return new Model(vertices, indices, surfaces, pointLights, centerOfMass);
}

void ObjLoader::loadPointLight(tinyobj::mesh_t& mesh, tinyobj::attrib_t& vertexAttributes, std::string name, std::vector<PointLight>& pointLights)
{

	const uint32_t indexCount = 3 * 2 * 6; // Cube vertices
	tinyobj::index_t indices[indexCount];
	for (size_t i = 0; i < indexCount; i++)
	{
		indices[i] = mesh.indices[i];
	}


	glm::vec3 position = glm::vec3(0.0);
	for (size_t i = 0; i < indexCount; i++)
	{
		position += glm::vec3(
			-vertexAttributes.vertices[3 * indices[i].vertex_index + 0],
			vertexAttributes.vertices[3 * indices[i].vertex_index + 2],  //Swap y and z axes, because z is up, not y
			vertexAttributes.vertices[3 * indices[i].vertex_index + 1]
		);
	}
	position /= indexCount;

	PointLight pointLight = {};
	pointLight.pos = glm::vec4(position, 1.0);

	// Parse object name.

	char delimiter = '.';
	std::vector<std::string> tokens = split(name, delimiter);
	if (tokens.size() < 4)
	{
		std::cout << "Light: " << name.c_str() << " cannot be parsed." << std::endl;
		return;
	}
	//pointLight.color = glm::vec4(0.0);
	//pointLight.color.r = stringToFloat(tokens[0]);
	//pointLight.color.g = stringToFloat(tokens[1]);
	//pointLight.color.b = stringToFloat(tokens[2]);
	//pointLight.color.a = stringToFloat(tokens[3]);

	pointLight.color.a *= 10.0; // test
	pointLight.color.r = 252.0 / 255.0;
	pointLight.color.g = 156.0 / 255.0;
	pointLight.color.b = 84.0 / 255.0;

	pointLights.push_back(pointLight);
}
void ObjLoader::loadMesh(tinyobj::mesh_t& mesh, tinyobj::attrib_t& vertexAttributes, std::vector<Vertex>& vertices, std::vector<Index>& indices, std::vector<Surface>& surfaces, std::unordered_map<Vertex, uint32_t>& vertexMap)
{
	uint32_t index_offset = 0;
	for (uint32_t f = 0; f < mesh.num_face_vertices.size(); f++)
	{
		loadFace(mesh, vertexAttributes, vertexMap, vertices, indices, surfaces, f, index_offset);

	};
}

Vertex ObjLoader::loadVertex(tinyobj::attrib_t& vertexAttributes, uint32_t materialIndex, tinyobj::index_t index)
{
	glm::vec3 pos = {
		vertexAttributes.vertices[3 * index.vertex_index + 0],
		-vertexAttributes.vertices[3 * index.vertex_index + 1],
		vertexAttributes.vertices[3 * index.vertex_index + 2]  //Swap y and z axes, because z is up, not y
	};

	glm::vec3 normal = {
		vertexAttributes.normals[3 * index.normal_index + 0],
		-vertexAttributes.normals[3 * index.normal_index + 1],
		vertexAttributes.normals[3 * index.normal_index + 2]//Swap y and z axes, because z is up, not y
	};

	glm::vec2 uv;
	if (2 * index.texcoord_index + 1 < vertexAttributes.texcoords.size())
	{
		uv = {
			vertexAttributes.texcoords[2 * index.texcoord_index + 0],
			vertexAttributes.texcoords[2 * index.texcoord_index + 1]
		};
	}
	else
	{
		uv = glm::vec2(0, 0);
	}
	int matIdx = materialIndex + materialIndexOffset;

	return Vertex(pos, glm::normalize(pos), uv, normal, matIdx, glm::vec3(0.0), glm::vec3(0.0));
}

void ObjLoader::loadFace(
	tinyobj::mesh_t& mesh,
	tinyobj::attrib_t& vertexAttributes,
	std::unordered_map<Vertex, uint32_t>& vertexMap,
	std::vector<Vertex>& vertices,
	std::vector<Index>& indices,
	std::vector<Surface>& surfaces,
	uint32_t faceIndex,
	uint32_t& index_offset)
{
	uint32_t fv = mesh.num_face_vertices[faceIndex];
	int matIdx = mesh.material_ids[faceIndex];
	Surface surf{};
	surf.startIndex = indices.size();
	surf.numIndices = fv;
	surf.mat = materialManager->materials[matIdx + materialIndexOffset];
	surf.matIndex = matIdx + materialIndexOffset;

	if (!surfaces.empty() && surfaces.back().matIndex == surf.matIndex)
	{
		surfaces.back().numIndices += fv;
	}
	else
	{
		surfaces.push_back(surf);
	}

	for (size_t v = 0; v < fv; v++)
	{
		tinyobj::index_t index = mesh.indices[index_offset + v];

		// Get vertex
		Vertex vert = loadVertex(vertexAttributes, matIdx, index);

		// Set vertex and index
		if (vertexMap.count(vert) == 0)
		{
			vertexMap[vert] = vertexMap.size();
			vertices.push_back(vert);
		}
		indices.push_back(vertexMap[vert]);
	}
	index_offset += fv;
}

void ObjLoader::calculateTangentAndBitangent(std::vector<Vertex>& vertices, std::vector<Index>& indices)
{
	for (size_t i = 0; i < indices.size() - 2; i += 3)
	{



		// Shortcuts for vertices
		glm::vec3 v0 = vertices[indices[i + 0]].pos;
		glm::vec3 v1 = vertices[indices[i + 1]].pos;
		glm::vec3 v2 = vertices[indices[i + 2]].pos;

		// Shortcuts for UVs
		glm::vec2 uv0 = vertices[indices[i + 0]].uvCoord;
		glm::vec2 uv1 = vertices[indices[i + 1]].uvCoord;
		glm::vec2 uv2 = vertices[indices[i + 2]].uvCoord;

		glm::vec3 N =     vertices[indices[i + 0]].normal
						+ vertices[indices[i + 1]].normal
						+ vertices[indices[i + 2]].normal;

		N = glm::normalize(N);


		// Following icg - slides

		// Edges of the triangle : position delta
		glm::vec3 dp1 = v1 - v0;
		glm::vec3 dp2 = v2 - v0;

		// UV delta
		glm::vec2 duv1 = uv1 - uv0;
		glm::vec2 duv2 = uv2 - uv0;



		glm::vec3 dp2perp = glm::cross(dp2, N);
		glm::vec3 dp1perp = glm::cross(N, dp1);
		glm::vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
		glm::vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
		float invdet = 1 / dot(dp1, dp2perp);

		T *= invdet;
		B *= invdet;

		vertices[indices[i + 0]].tangent = T;
		vertices[indices[i + 1]].tangent = T;
		vertices[indices[i + 2]].tangent = T;

		vertices[indices[i + 0]].bitangent = B;
		vertices[indices[i + 1]].bitangent = B;
		vertices[indices[i + 2]].bitangent = B;

	}

	for (Vertex vertex : vertices)
	{
		vertex.tangent = glm::normalize(vertex.tangent);
		vertex.bitangent = glm::normalize(vertex.bitangent);
	}
}