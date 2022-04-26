#pragma once
#include "Context.h"
#include "ShaderStructs.h"



class DrawGeom;

class DrawSurf
{
public:
	enum Opacity
	{
		OPAQUE,
		CUT_OUT
	};
	DrawSurf(
		DrawGeom* geom,
		uint32_t firstIndex,
		uint32_t numIndices,
		uint32_t vertexBufferOffset);
	DrawSurf(DrawGeom* geom, uint32_t firstIndex, uint32_t numIndices, uint32_t vertexBufferOffset, Opacity opacity);
	~DrawSurf();


	DrawGeom* geom;
	uint32_t firstIndex;
	uint32_t numIndices;
	uint32_t vertexBufferOffset;
	Opacity opacity;

private:

};

