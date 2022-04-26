#include "DrawInst.h"


DrawInst::DrawInst(InstanceShaderData shaderData, DrawGeom* geom)
{
	this->shaderData = shaderData;
	this->shaderData.oldModelMat = shaderData.modelMat;
	this->geometry = geom;
}

DrawInst::DrawInst()
{
}


DrawInst::~DrawInst()
{
}
