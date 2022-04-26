#version 460
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_ray_query : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "render_common.glsl"
#include "rt_common.glsl"
#include "brdf.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform PFC
{
	PerFrameConstants d;
} pfc;

struct PushConstants
{
	uint enablePathTracing;
	uint placeholder;
};
layout(push_constant) uniform PushStruct {PushConstants pc;};

layout (input_attachment_index = 0, binding = 1) uniform subpassInput samplerAlbedo;
layout (input_attachment_index = 1, binding = 2) uniform subpassInput samplerNormal;
layout (input_attachment_index = 2, binding = 3) uniform subpassInput samplerRoughnessMetalicity;
layout (input_attachment_index = 3, binding = 4) uniform subpassInput samplerPos;

layout(std140, binding = 5) buffer DESC_DATA
{
	InstanceDescriptor d[INSTANCE_COUNT];
} inst;

layout(binding = 6, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 7, set = 0, rgba32f) uniform image2D image;

layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; }; // Positions of an object
layout(buffer_reference, scalar) buffer Indices {ivec3 i[]; }; // Triangle indices


void calculateShadingVectorsForPointLight(inout ShadingData data, vec3 lightPosition)
{
	data.view			= normalize(pfc.d.camPos.xyz - data.worldPos);
	data.lightVector	= normalize(lightPosition - data.worldPos);
	data.halfWayVector	= normalize(data.lightVector + data.view);
	data.dotLH			= clamp(dot(data.lightVector, data.halfWayVector), 0.0, 1.0);
	data.dotNV			= abs(dot(data.normal, data.view) + 0.0001);
	data.dotNH			= clamp(dot(data.normal, data.halfWayVector), 0.0, 1.0);
	data.dotNL			= clamp(dot(data.normal, data.lightVector), 0.0, 1.0);
}

void calculateShadingVectorsForDirectionalLight(inout ShadingData data, vec3 lightDirection)
{
	data.view			= normalize(pfc.d.camPos.xyz - data.worldPos);
	data.lightVector	= normalize(lightDirection);
	data.halfWayVector	= normalize(data.lightVector + data.view);
	data.dotLH			= clamp(dot(data.lightVector, data.halfWayVector), 0.0, 1.0);
	data.dotNV			= abs(dot(data.normal, data.view) + 0.0001);
	data.dotNH			= clamp(dot(data.normal, data.halfWayVector), 0.0, 1.0);
	data.dotNL			= clamp(dot(data.normal, data.lightVector), 0.0, 1.0);
}

void castShadowRay(inout vec4 outColor, ShadingData data)
{
	vec3 origin = data.worldPos;
	vec3 direction = normalize(pfc.d.directionalLight.xyz);
	float tMin = 0.08; // small number
	float tMax = 100.0; // large number
	rayQueryEXT rayQuery;
	bool hit;
	rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, origin, tMin, direction, tMax);
	while(rayQueryProceedEXT(rayQuery)){}

	if(rayQueryGetIntersectionTypeEXT(rayQuery, true) == gl_RayQueryCommittedIntersectionTriangleEXT)
	{
		outColor.rgb *= 0.2;
	}
}

ShadingData prepareShadingData()
{
	ShadingData data;
	vec2 roughnessMetalicity = subpassLoad(samplerRoughnessMetalicity).rg;
	data.metalicity			= roughnessMetalicity.g;
	data.roughness			= clamp(roughnessMetalicity.r, 0.01, 1.0);
	data.normal				= 2.0*subpassLoad(samplerNormal).rgb - 1.0;
	data.diffuse_albedo		= subpassLoad(samplerAlbedo).rgb;
	data.fresnel_0			= mix(vec3(0.02f), data.diffuse_albedo, roughnessMetalicity.g);
	//data.roughness			= clamp(roughnessMetalicity.r, 0.01, 1.0);
	data.PI					= 3.141592f;
	data.diffuse_color		= (1.0 - data.metalicity) * data.diffuse_albedo;
	data.specular_color		= vec3(1.0);
	vec4 posData			= subpassLoad(samplerPos);

	//if(posData.a == 0.0)
	//{
	//	discard;
	//}

	data.worldPos			= posData.rgb;//calculateWorldPosition(subpassLoad(samplerDepth).r).xyz;
	return data;
}


void main()
{
	

	vec4 posData = subpassLoad(samplerPos);
	if(posData.a == 0.0)
	{
		outColor.a = 1.0;
		outColor.rgb = vec3(0.1,0.1,0.2);
		vec2 uv = vec2(gl_FragCoord.x/pfc.d.width, gl_FragCoord.y/pfc.d.height);
		vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0f, 1.0, 1.0);
		vec4 viewSpacePosition = pfc.d.inverseProjection* clipSpacePosition;
		vec4 worldSpacePosition = pfc.d.inverseView * viewSpacePosition;
		outColor.rgb += vec3(1.0,0.4,0.1)*0.2*(1.0+worldSpacePosition.y);
		//outColor.rgb*=0.8;
		return;
	}

	//outColor = vec4(0.0,0.0,1.0,1.0);return;
	// Prepare shading data
	ShadingData data = prepareShadingData();
	///calculateShadingVectorsForDirectionalLight(data, pfc.d.directionalLight.xyz);
	//calculateShadingVectorsForDirectionalLight(data, vec3(0.0,-2.0,-1.0));
	calculateShadingVectorsForDirectionalLight(data, pfc.d.directionalLight.xyz);
	float lightIntensity = pfc.d.directionalLight.w * 5.0;
	//vec3 ligthColor = vec3(1.0,0.8,0.5);
	vec3 ligthColor = vec3(1.0,0.8,0.6);
	outColor.a = 1.0;

	// Evaluate Shading
	vec3 brdf = evaluateBRDF(data);
	outColor.rgb = brdf*data.dotNL*lightIntensity * ligthColor.rgb;

	castShadowRay(outColor, data);

	// Ambient term
	outColor.rgb+= data.diffuse_albedo*0.08;
	//outColor.rgb*=0.0;
	//outColor.rgb= data.diffuse_albedo;
	
	//outColor.rgb= data.normal;
	//outColor.r = data.roughness;
	//outColor.g = data.metalicity;
	//outColor.b = 0.0;
	if(pc.enablePathTracing == 1)
	{
		outColor = vec4(0.0);
		for(int x = -4; x<=4;x++)
		{
			for(int y = -4; y<=4;y++)
			{
				float d = length(vec2(x,y));
				float coef = 0.1/(2*d*d+1);
				outColor += coef*imageLoad(image, ivec2(gl_FragCoord.x+x,gl_FragCoord.y+y));
			}
		}
	}


	//debug
	//outColor.rgb= data.diffuse_albedo;
	//outColor.rgb = vec3(data.metalicity, data.roughness, 0);
	//outColor.rgb = 0.5*data.normal.rgb + 0.5;
}