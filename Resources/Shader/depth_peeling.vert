#version 460 
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_ray_query : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
//#extension GL_ARB_separate_shader_objects : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_atomic_float : require

#include "GlobalValues.glsl"
#include "render_common.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 inUVCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;
layout(location = 6) in uint materialIndex;

layout(binding = 0) uniform PFC
{
	DepthPeelingConstants d;
} pfc;

layout (location = 0) out vec3 model_pos;

layout (location = 1) out mat4 VP;

void main()
{

    model_pos = pos;

    mat4 PV = pfc.d.projection * pfc.d.view;
    gl_Position = PV * vec4(pos, 1.0);

    VP = pfc.d.view * pfc.d.projection;
}