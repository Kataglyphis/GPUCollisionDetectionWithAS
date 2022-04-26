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

#define MAX_FRAGMENT_COUNT 8
#define INSERTION_RATE 8

layout (binding = 1) buffer CountersBuffer
{
    Counter counters[];
};

layout (binding = 2) buffer LinkedList
{
    Node nodes[];
};

layout (binding = 4) buffer ParticleBuffer
{
    Particle particles[];
};

layout (binding = 3, r32ui) uniform uimage2D headIndexImage;

layout(location = 0) in vec2 inUV;

void main()
{

    Node fragments[MAX_FRAGMENT_COUNT];
    int tmp_count = 0;

    uint nodeIdx = imageLoad(headIndexImage, ivec2(gl_FragCoord.xy)).r;

    while (nodeIdx != 0xffffffff && tmp_count < MAX_FRAGMENT_COUNT)
    {
        fragments[tmp_count] = nodes[nodeIdx];
        nodeIdx = fragments[tmp_count].next;
        ++tmp_count;
    }
    
    //Do the insertion sort
    for (uint i = 1; i < tmp_count; ++i)
    {
        Node insert = fragments[i];
        uint j = i;
        while (j > 0 && insert.position.z < fragments[j - 1].position.z)
        {
            fragments[j] = fragments[j-1];
            --j;
        }
        fragments[j] = insert;
    }   

    for (uint i = 0; i < tmp_count; ++i)
    {
        // the current element should always be included 
        int current_index = int(atomicAdd(counters[0].particle_count, 1));
        particles[current_index].position       = fragments[i].position;

        if(i == tmp_count - 1) {
            // stop at last fragment
            continue;

        } else {
            
            float distance_of_fragments = length(fragments[i+1].position - fragments[i].position);
            vec4 insert_dir             = normalize(fragments[i+1].position - fragments[i].position); 

            for (uint j = 0; j < INSERTION_RATE; j++) {
                
                vec4 insert_position                = fragments[i].position + (distance_of_fragments/INSERTION_RATE) * j
                                                        * insert_dir;

                int current_index                   = int(atomicAdd(counters[0].particle_count, 1));
                particles[current_index].position   = insert_position;

            }

        }
    }
    

}