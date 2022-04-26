#include "MaterialTextureManager.h"
#include "PipelineToolKit.h"




MaterialTextureManager::MaterialTextureManager(ResourceManager* rm)
{
	this->rm = rm;
}




uint32_t MaterialTextureManager::addImage(std::string texture_path, TextureFormat textureFormat)
{
	if (textureInfoMap.count(texture_path) == 0)
	{
		TextureInfo textureInfo = { texture_path, textureFormat };
		textureInfoMap[texture_path] = texturesInfos.size();
		texturesInfos.push_back(textureInfo);
	}
	return textureInfoMap[texture_path];
}

void MaterialTextureManager::uploadImages()
{
	// Get Images
	std::vector<StbImageInfo> imageInfos;
	uint32_t texCount = texturesInfos.size();
	//texCount = 1; // Debug
	for (uint32_t i = 0; i < texCount; i++)
	{
		int height;
		int width;
		int channels;
	
		StbImageInfo imageInfo = {};
		imageInfo.pPixels = stbi_load(texturesInfos[i].path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		//assert(channels == 4);
	
		uint32_t mipLevels = std::floor(std::log2(std::max(width, height))) + 1;
	
		VkImageCreateInfo imageCreateInfo = vk_default::textureImageCreateInfo();
		imageCreateInfo.format = (VkFormat) texturesInfos[i].format;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = mipLevels;
		imageCreateInfo.arrayLayers = 1;
	
		imageInfo.size = height * width * 4;
		imageInfo.imgCreateInfo = imageCreateInfo;
		imageInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	
		imageInfos.push_back(imageInfo);
	}
	
	// Upload & generate mipmaps
	rm->immediatUploadStbImages(imageInfos, textures);
	


	// Free Memory
	for (uint32_t i = 0; i < imageInfos.size(); i++)
	{
		stbi_image_free(imageInfos[i].pPixels);
	}
}

std::vector<Image> MaterialTextureManager::getImages()
{
	return textures;
}

