#pragma once
#include <stdint.h>
#include <tiny_obj_loader.h>
#include <vector>
#include <cstdint>
#include "MaterialTextureManager.h"
#include "ResourceManager.h"
#include "ShaderStructs.h"

class MaterialManager
{
public:
	MaterialManager(ResourceManager* rm, MaterialTextureManager* textureManager);
	~MaterialManager();

	uint32_t addMaterial(tinyobj::material_t materials, std::string textureBaseDir);

	void uploadMaterials();

	Buffer getMaterialBuffer();
	uint32_t getMaterialCount();
	std::vector<Material>	materials;

private:
	ResourceManager*		rm;
	MaterialTextureManager* textureManager;

	Buffer					materialBuffer;

};