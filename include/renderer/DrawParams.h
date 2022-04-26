#pragma once
#include "vulkan/vulkan.h"
#include "RasterizationStage.h"

class DrawGeom;
class DrawInst;

struct DrawParams
{
	VkBuffer*	pVertexBufferScene;
	VkBuffer	indexBufferScene;
	uint32_t	indexCountScene;


	std::vector<DrawGeom*> geom;
	std::vector<DrawInst*> inst;


	VkBuffer*	pVertexBufferParticles;
	uint32_t	vertexCountParticles;
	
	VkBuffer*	pVertexBufferSceneParticles;
	uint32_t	vertexCountSceneParticles;
};