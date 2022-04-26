#include "DrawSurf.h"


DrawSurf::DrawSurf(DrawGeom* geom, uint32_t firstIndex, uint32_t numIndices, uint32_t vertexBufferOffset, Opacity opacity)
{
	this->geom = geom;
	this->firstIndex = firstIndex;
	this->numIndices = numIndices;
	this->vertexBufferOffset = vertexBufferOffset;
	this->opacity = opacity;
}

DrawSurf::~DrawSurf()
{
}