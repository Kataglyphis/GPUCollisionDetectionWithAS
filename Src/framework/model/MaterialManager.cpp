#include "MaterialManager.h"



uint32_t MaterialManager::addMaterial(tinyobj::material_t tinyObjMat, std::string textureBaseDir)
{
	// Values for empty slot
	Material mat = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
	// Write texture index to material slot
	if(tinyObjMat.diffuse_texname	!= "") mat.tex_idx_diffuse	= textureManager->addImage(	textureBaseDir + "/" + tinyObjMat.diffuse_texname	, TEXTURE_FORMAT_DIFFUSE_ALBEDO);
	if(tinyObjMat.bump_texname		!= "") mat.tex_idx_normal	= textureManager->addImage(	textureBaseDir + "/" + tinyObjMat.bump_texname		, TEXTURE_FORMAT_NORMAL);
	if(tinyObjMat.specular_texname	!= "") mat.tex_idx_specular	= textureManager->addImage(	textureBaseDir + "/" + tinyObjMat.specular_texname	, TEXTURE_FORMAT_SPECULAR);
	if(tinyObjMat.alpha_texname		!= "") mat.tex_idx_coverage	= textureManager->addImage(	textureBaseDir + "/" + tinyObjMat.alpha_texname		, TEXTURE_FORMAT_COVERAGE);
	if(tinyObjMat.ambient_texname	!= "") mat.tex_idx_ambient	= textureManager->addImage(	textureBaseDir + "/" + tinyObjMat.ambient_texname	, TEXTURE_FORMAT_EMISSIVE);
	materials.push_back(mat);
	return materials.size()-1;
}


void MaterialManager::uploadMaterials()
{
	rm->immediateCreateAndUploadDeviceBuffer(materialBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, materials);
}

Buffer MaterialManager::getMaterialBuffer()
{
	return materialBuffer;
}

uint32_t MaterialManager::getMaterialCount()
{
	return materials.size();
}



MaterialManager::MaterialManager(ResourceManager* rm ,MaterialTextureManager* textureManager)
{
	this->rm = rm;
	this->textureManager = textureManager;
}

MaterialManager::~MaterialManager()
{
}


