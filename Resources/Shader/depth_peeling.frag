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

layout (early_fragment_tests) in;

layout(binding = 0) uniform PFC
{
	DepthPeelingConstants d;
} pfc;

layout (binding = 1) buffer CountersBuffer
{
    Counter counters[];
};

layout (binding = 2) buffer LinkedList
{
    Node nodes[];
};

layout (binding = 3, r32ui) uniform coherent uimage2D headIndexImage;

layout (binding = 4) buffer ParticleBuffer
{
    Particle particles[];
};

layout (location = 0) in vec3 world_pos;
layout (location = 1) in mat4 VP;

void main()
{
    
    // Increase the node count
    uint nodeIdx = atomicAdd(counters[0].count, 1);

    // Exchange new head index and previous head index
    uint prevHeadIdx = imageAtomicExchange(headIndexImage, ivec2(gl_FragCoord.xy), nodeIdx);

    //Store node data
    //Check LinkedListSBO is full
    if (nodeIdx < uint(counters[0].maxNodeCount))
    {
        nodes[nodeIdx].position = vec4(world_pos,1.0f);
        nodes[nodeIdx].depth    = gl_FragCoord.z;
        nodes[nodeIdx].next     = prevHeadIdx;
    }
   

}