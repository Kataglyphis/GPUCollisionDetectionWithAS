#pragma once
#include <cstdint>
#include <string>
#include "ResourceManager.h"
#include <unordered_map>


struct TextureInfo
{
	std::string		path;
	TextureFormat	format;
};


class MaterialTextureManager
{
public:
	MaterialTextureManager(ResourceManager* rm);

	uint32_t addImage(std::string texture_path, TextureFormat texFormat);

	void uploadImages();

	std::vector<Image> getImages();

private:
	ResourceManager*				rm;

	std::unordered_map<std::string, uint32_t> textureInfoMap;
	std::vector<TextureInfo>		texturesInfos;
	std::vector<Image>				textures;
};

