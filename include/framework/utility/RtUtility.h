#pragma once
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include "DrawGeom.h"

namespace vk_rt
{
	static VkTransformMatrixKHR getTransformMatrix(glm::mat4 mat)
	{
		VkTransformMatrixKHR transform;
		transform.matrix[0][0] = mat[0][0];
		transform.matrix[0][1] = mat[1][0];
		transform.matrix[0][2] = mat[2][0];
		transform.matrix[0][3] = mat[3][0];

		transform.matrix[1][0] = mat[0][1];
		transform.matrix[1][1] = mat[1][1];
		transform.matrix[1][2] = mat[2][1];
		transform.matrix[1][3] = mat[3][1];

		transform.matrix[2][0] = mat[0][2];
		transform.matrix[2][1] = mat[1][2];
		transform.matrix[2][2] = mat[2][2];
		transform.matrix[2][3] = mat[3][2];

		return transform;
	}


	static std::vector<uint32_t> getInstanceShaderBindingTableRecordOffsets(std::vector<DrawInst*> instances, uint32_t rayTypes)
	{
		std::vector<uint32_t> sbtInstanceOffsets(instances.size());
		uint32_t currentOffset = 0;
		DrawGeom* lastGeom = nullptr;
		for (uint32_t i = 0; i < instances.size(); i++)
		{
			if (instances[i]->geometry == lastGeom)
			{
				sbtInstanceOffsets[i] = sbtInstanceOffsets[i - 1];
			}
			else
			{
				sbtInstanceOffsets[i] = currentOffset;
				currentOffset += instances[i]->geometry->surfs.size() * rayTypes;
				lastGeom = instances[i]->geometry;
			}
		}
		return sbtInstanceOffsets;
	}


}