#include "Config.h"

GVar g_enable_path_tracing{ "enable path tracing", false, GVAR_BOOL, GVAR_RENDER };

GVar g_enable_classic_physics{ "enable classic physics", false, GVAR_BOOL, GVAR_PHYSICS };
GVar g_grid_offset{ "grid offset", {.0f,.0f,.0f}, GVAR_VEC3, GVAR_PHYSICS };
GVar g_grid_size_x{ "grid size x", 20U, GVAR_UINT, GVAR_PHYSICS };
GVar g_grid_size_y{ "grid size y", 20U, GVAR_UINT, GVAR_PHYSICS };
GVar g_grid_size_z{ "grid size z", 20U, GVAR_UINT, GVAR_PHYSICS };
GVar g_reset_grid{ "reset grid", false, GVAR_EVENT, GVAR_PHYSICS };

GVar g_explosion_center{ "explosion center", {0.f,0.f,0.f} , GVAR_VEC3, GVAR_PHYSICS };
GVar g_explosion_strength{ "explosion strength", 0.5f, GVAR_FLOAT, GVAR_PHYSICS };
GVar g_enable_explosion{ "enable explosion", false, GVAR_BOOL, GVAR_PHYSICS };

GVar g_gravity_strength{ "gravity strength", 0.1f, GVAR_FLOAT, GVAR_PHYSICS };
GVar g_enable_gravity{ "enable gravity", true, GVAR_BOOL, GVAR_PHYSICS };
GVar g_physics_timings{ "physics timings: %.3lf ms", .0f, GVAR_DISPLAY_VALUE, GVAR_PHYSICS };


std::vector<GVar*> gVars = {
	&g_enable_path_tracing,
	&g_enable_classic_physics,
	&g_grid_offset,
	&g_grid_size_x,
	&g_grid_size_y,
	&g_grid_size_z,
	&g_reset_grid,
	&g_explosion_center,
	&g_explosion_strength,
	&g_enable_explosion,
	&g_gravity_strength,
	&g_enable_gravity,
	&g_physics_timings
};