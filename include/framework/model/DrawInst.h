#pragma once
#include "glm/glm.hpp"

class DrawGeom;


struct InstanceShaderData
{
	glm::mat4 modelMat;
	glm::mat4 oldModelMat;
	glm::vec4 position;
	glm::vec4 rotation;
	glm::vec4 velocity;
	glm::vec4 angularVelocity;
	glm::vec4 scale;
	glm::vec4 torque;
	glm::vec4 force;

	uint32_t	collisionCount;
	uint32_t	state;
	uint32_t	placeholder2;
	uint32_t	placeholder3;
};

class DrawInst
{
public:
	DrawInst(InstanceShaderData shaderData, DrawGeom* geom);
	DrawInst();
	~DrawInst();

	InstanceShaderData shaderData;
	uint32_t customIntersectionIndex;
	DrawGeom* geometry;
private:

};

