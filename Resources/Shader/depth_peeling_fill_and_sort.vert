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

layout(location = 0) out vec2 outUV;

layout(binding = 0) uniform PFC
{
	DepthPeelingConstants d;
} pfc;

void main()
{
	outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}