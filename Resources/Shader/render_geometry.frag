#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "render_common.glsl"

layout(location = 0) flat in uint materialIndex;

layout (location = 1) flat in int drawId;

layout(location = 2) in VS_INTERPOLATET
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



layout (location = 0) out vec4 outAlbedo;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outRoughnessMetalicity;
layout (location = 3) out vec4 outPos;


layout(buffer_reference, scalar) buffer Materials {Material i[]; }; 
layout( push_constant ) uniform constants
{
	uint64_t instanceAddr;
	uint64_t materialAddr;
} PushConstants;

layout(binding = 0) uniform PFC
{
	PerFrameConstants d;
} pfc;

layout(binding = 1) uniform sampler smp;
layout(binding = 2) uniform texture2D tex[TEXTURE_COUNT];

layout(std140, binding = 3) buffer MAT
{
	Material d[MATERIAL_COUNT];
} mat;


vec3 calculateNormal(vec4 normalData)
{
	vec3 N = normalize(vs_out.fragNormal);
	vec3 T = normalize(vs_out.fragTangent);
	T = normalize(T - dot(T, N) * N);
	vec3 B = -normalize(cross(N, T));

	mat3 TBN = mat3(T,B,N);

	return normalize(TBN*(2.0 * normalData.rgb - 1.0));
}


void writeMaterialDataToGBuffer()
{
	Materials materials = Materials(PushConstants.materialAddr);
	Material material = materials.i[drawId];
	//material = mat.d[materialIndex];

	// Cut out geometry
	if(material.tex_idx_coverage != MAX_UINT)
	{
		float transparency = texture(sampler2D(tex[material.tex_idx_coverage], smp), vs_out.uvCoord).a;
		if(transparency < 0.50)
		{
			discard;
		}
	}
	// Default values
	vec4 normalData		= vec4(0.0, 0.0, 1.0, 0.0);
	vec4 specularData	= vec4(0.0,0.2,0.0,0.0);
	vec4 diffuseData	= vec4(1.0,0.0,0.0,1.0);
	// Read textures
	if(material.tex_idx_diffuse!=MAX_UINT)diffuseData		= texture(sampler2D(tex[material.tex_idx_diffuse], smp), vs_out.uvCoord);
	if(material.tex_idx_normal!=MAX_UINT)normalData			= texture(sampler2D(tex[material.tex_idx_normal], smp), vs_out.uvCoord);
	if(material.tex_idx_specular!=MAX_UINT)specularData		= texture(sampler2D(tex[material.tex_idx_specular], smp), vs_out.uvCoord);

	float metalicity = specularData.b;
	float roughness	 = specularData.g;

	// Write to gbuffer
	//outAlbedo = vec4(vs_out.uvCoord,1.0,0.0);
	outNormal.rgb = 0.5*calculateNormal(normalData) + 0.5;
	outRoughnessMetalicity.rg = vec2(roughness, metalicity);
	outPos = vec4(vs_out.worldPos.rgb, 1.0);
	outAlbedo = diffuseData;


	//outNormal.rgb = normalData.rgb;
	// Hardcoded water fix
	if(material.tex_idx_diffuse==70 && outAlbedo.r < 0.01 && outAlbedo.g < 0.01 && outAlbedo.b < 0.01)
	{
		outAlbedo.a = 1.0;
		outAlbedo.rgb = 2*texture(sampler2D(tex[0], smp), vs_out.uvCoord*2.0).rgb+vec3(0.0,0.0,0.02)*outPos.length();
		normalData = texture(sampler2D(tex[1], smp), vs_out.uvCoord);
		outNormal.rgb = 0.5*calculateNormal(normalData) + 0.5;
		outRoughnessMetalicity.rg = vec2(0.04, 0.0);
		return;
	}
	//outNormal.rgb = vs_out.fragNormal;
	//outNormal.rgb = normalData.rgb;

}

void main()
{
	//int x = int(gl_PrimitiveID);
	//outAlbedo = vec4(0.1*(x%10), 0.1*((x/10)%10),  0.1*((x/100)%10), 1.0);




	writeMaterialDataToGBuffer();
}