#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#include "rt_common.glsl"

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main()
{
	//prd.hitValue = vec3(0.5,0.5,0.5);
	prd.hitValue = vec3(1.0,0.4,0.1)*0.4+vec3(0.2,0.3,0.9);
}