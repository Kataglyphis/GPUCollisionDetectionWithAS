#version 460
#extension GL_GOOGLE_include_directive : enable
//#extension GL_ARB_seperate_objects : enable
//#extension GL_KHR_vulkan_glsl : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#include "render_common.glsl"

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 inUVCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;
layout(location = 6) in uint materialIndex;


layout( push_constant ) uniform constants
{
	uint64_t instanceAddr;
	uint64_t materialAddr;
} PushConstants;

layout(buffer_reference, scalar) buffer Instances {Instance i[]; }; 



layout(location = 0) flat out uint fragMaterialIndex;
layout (location = 1) flat out int drawId;
layout(location = 2) out VS_INTERPOLATET
{
	vec3 viewVector;
	vec2 uvCoord;
	vec3 lightVector;
	vec4 worldPos;

	vec3 fragNormal;
	vec3 fragTangent;
	vec3 fragBitangent;
	vec4 modelPos;

} vs_out;



layout(binding = 0) uniform PFC
{
	PerFrameConstants d;
} pfc;



void main()
{

	//int x = int(gl_InstanceIndex);
	//vec3 offset = vec3(0.1*(x%10), 0.1*((x/10)%10),  0.1*((x/100)%10));
	//vec3 offset = vec3(0.1*gl_InstanceIndex);
	Instances instances = Instances(PushConstants.instanceAddr);
	mat4 modelMat = instances.i[gl_InstanceIndex].modelMat;

	gl_Position = pfc.d.projection * pfc.d.view * modelMat * vec4(pos, 1.0);

	//vs_out.worldPos = pfc.d.model * vec4(pos, 1.0);

	vs_out.worldPos = modelMat * vec4(pos, 1.0); //
	vs_out.modelPos = vec4(pos, 1.0);
	
	vs_out.uvCoord = vec2(inUVCoord.x,1.0-inUVCoord.y); // civ
	fragMaterialIndex = materialIndex;
	
	vs_out.fragNormal =  normalize(vec3(modelMat * vec4(inNormal, 0)));
	vs_out.fragTangent = normalize(vec3(modelMat * vec4(inTangent, 0)));
	vs_out.fragBitangent = normalize(vec3(modelMat * vec4(inBitangent, 0)));
	
	//vs_out.lightVector = normalize(pfc.d.lightPosition.xyz - vs_out.worldPos.xyz);
	vs_out.viewVector = normalize(pfc.d.camPos.xyz - vs_out.worldPos.xyz);
	
	drawId = gl_DrawID;
}