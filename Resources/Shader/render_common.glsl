

#ifndef TEXTURE_COUNT
#define TEXTURE_COUNT 200
#endif

#ifndef MATERIAL_COUNT
#define MATERIAL_COUNT 200
#endif

#define MAX_UINT 0xFFFFFFFF

struct ShadingData
{
	vec3	view;				//V
	vec3	normal;				//N
	vec3	halfWayVector;		//H
	vec3	lightVector;		//L

	vec3	worldPos;

	vec3	specular_color;
	vec3	diffuse_color;
	vec3	ambient_color;

	
	vec3	fresnel_0;			// Color of specular reflection at 0 degree
	float	roughness;

	float	dotNV;
	float	dotLH;
	float	dotNH;
	float	dotNL;
	
	float	PI;
	float	metalicity;
	vec3	diffuse_albedo;
	vec3	emissive;

	vec3	surfaceNormal;
	mat3	TNB;
};

struct PerFrameConstants
{
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 inverseView;
	mat4 inverseProjection;
	vec4 directionalLight;
	vec4 camPos;

	uint width;
	uint height;
	uint cnt;
	uint placeholder2;

	mat4 particleModel;
};

struct Material
{
	uint tex_idx_diffuse;
	uint tex_idx_normal;
	uint tex_idx_specular;
	uint tex_idx_coverage;

	uint tex_idx_ambient;
	uint indexOffset;
	uint placeholder2;
	uint placeholder3;
};

struct Instance
{
	mat4 modelMat;
	mat4 oldModelMat;
	vec4 position;
	vec4 rotation;
	vec4 velocity;
	vec4 angularVelocity;
	vec4 scale;
	vec4 torque;
	vec4 force;

	uint	collisionCount;
	uint	state;
	uint	placeholder2;
	uint	placeholder3;
};


struct AccelerationStructureInstanceKHR
{
    //vec4 row1;
    //vec4 row2;
    //vec4 row3;
	mat3x4 mat;

    uint placeholder1;
    uint placeholder2;
    uint placeholder3;
    uint placeholder4;
};


struct VoxelGridEntry
{
	uint particle0;
	uint particle1;
	uint particle2;
	uint counter;
};

ivec3 getGridIndexVector(vec3 pos, vec3 gridOffset, float gridCellSize)
{
	//vec3 gridPos = -(vec3(pos.x, pos.y, pos.z) - gridOffset);
	vec3 gridPos = -pos - (gridOffset+vec3(-2.0,0.0,-2.0));
	ivec3 gridIndexVector;
	gridIndexVector.x = int(gridPos.x/gridCellSize);
	gridIndexVector.y = int(gridPos.y/gridCellSize);
	gridIndexVector.z = int(gridPos.z/gridCellSize);
	return gridIndexVector;
}

uint getGridIndex(ivec3 gridIndexVector, uint gridExtent)
{
	if(gridIndexVector.x < 0 || gridIndexVector.y < 0 || gridIndexVector.z < 0) return MAX_UINT;
	if(gridIndexVector.x >= gridExtent || gridIndexVector.y >= gridExtent || gridIndexVector.z >= gridExtent) return MAX_UINT;
	uint gridIndex = gridIndexVector.x*gridExtent*gridExtent;
	gridIndex += gridIndexVector.y*gridExtent;
	gridIndex += gridIndexVector.z;
	if(gridIndex >= gridExtent*gridExtent*gridExtent) return MAX_UINT;
	return gridIndex;
}