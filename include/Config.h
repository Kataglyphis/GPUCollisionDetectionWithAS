#pragma once
#include <string>
#include <glm/glm.hpp>
#include <vector>


// Gui Variable
union GVar_Val
{
	bool		v_bool;
	uint32_t	v_uint;
	float		v_float;
	float		v_vec3[3];

	GVar_Val(bool b)
	{
		v_bool = b;
	}
	GVar_Val(uint32_t i)
	{
		v_uint = i;
	}
	GVar_Val(float f)
	{
		v_float = f;
	}
	GVar_Val(float x, float y, float z)
	{
		v_vec3[0] = x;
		v_vec3[1] = y;
		v_vec3[2] = z;
	}
	glm::vec4 getVec4()
	{
		return glm::vec4(v_vec3[0], v_vec3[1], v_vec3[2], 0);
	};
	uint32_t bool32()
	{
		return v_bool ? 1:0;
	};
};

enum GVar_Type
{
	GVAR_EVENT,
	GVAR_BOOL,
	GVAR_FLOAT,
	GVAR_UINT,
	GVAR_VEC3,
	GVAR_DISPLAY_VALUE,
};

enum GVar_Cat
{
	GVAR_RENDER,
	GVAR_PHYSICS
};

struct GVar
{
	std::string name;
	GVar_Val val;
	GVar_Type type;
	GVar_Cat cat;
};

extern GVar g_enable_path_tracing;
extern GVar g_enable_classic_physics;
extern GVar g_explosion_center;
extern GVar g_explosion_strength;
extern GVar g_enable_explosion;
extern GVar g_gravity_strength;
extern GVar g_enable_gravity;

extern GVar g_grid_offset;
extern GVar g_grid_size_x;
extern GVar g_grid_size_y;
extern GVar g_grid_size_z;
extern GVar g_reset_grid;
extern GVar g_physics_timings;

extern std::vector<GVar*> gVars;