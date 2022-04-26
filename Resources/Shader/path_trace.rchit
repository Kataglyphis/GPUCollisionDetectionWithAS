#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : require

#include "render_common.glsl"
#include "rt_common.glsl"
#include "brdf.glsl"

//#define NUM_SAMPLES 4

#define RETURN_HASHED(A) prd.hitValue = vec3(0.1*(int((A))%10), 0.1*((int((A))/10)%10),  0.1*((int((A))/100)%10));return;
#define RETURN_COLOR(A) prd.hitValue = (A).rgb;return;




hitAttributeEXT vec2 attribs;
layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

layout(binding = 0) uniform PER_FRAME_CONSTANTS { PerFrameConstants pfc; };
layout(binding = 1) buffer instances {InstanceDescriptor inst[];};
layout(binding = 2, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 3) uniform sampler smp;
layout(binding = 4) uniform texture2D tex[TEXTURE_COUNT];
//layout(binding = 5, set = 0, rgba32f) uniform image2D image;

// Buffer references


layout(buffer_reference, scalar) buffer Indices {ivec3 d[]; };
layout(buffer_reference, scalar) buffer Vertices {Vertex d[]; };
layout(buffer_reference, scalar) buffer Materials {Material d[]; };


vec3 calculateNormal(vec4 normalData, vec3 geomNormal, vec3 geomTangent)
{
	vec3 N = normalize(geomNormal);
	vec3 T = normalize(geomTangent);
	T = normalize(T - dot(T, N) * N);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T,B,N);

	return normalize(TBN*(2.0 * normalData.rgb - 1.0));
}

//void calculateShadingVectorsForPointLight(inout ShadingData data, vec3 lightPosition)
//{
//	//data.view			= normalize(pfc.camPos.xyz - data.worldPos);
//	data.lightVector	= normalize(lightPosition - data.worldPos);
//	data.halfWayVector	= normalize(data.lightVector + data.view);
//	data.dotLH			= clamp(dot(data.lightVector, data.halfWayVector), 0.0, 1.0);
//	data.dotNV			= abs(dot(data.normal, data.view) + 0.0001);
//	data.dotNH			= clamp(dot(data.normal, data.halfWayVector), 0.0, 1.0);
//	data.dotNL			= clamp(dot(data.normal, data.lightVector), 0.0, 1.0);
//}

void calculateShadingVectorsForDirectionalLight(inout ShadingData data, vec3 lightDirection)
{
	//data.view			= normalize(pfc.camPos.xyz - data.worldPos);
	data.lightVector	= normalize(lightDirection);
	data.halfWayVector	= normalize(data.lightVector + data.view);
	//data.halfWayVector	= vec3(1.0,0.0,0.0);
	data.dotLH			= clamp(dot(data.lightVector, data.halfWayVector), 0.0, 1.0);
	data.dotNV			= abs(dot(data.normal, data.view) + 0.0001);
	//data.dotNV			= 0.5;
	data.dotNH			= clamp(dot(data.normal, data.halfWayVector), 0.0, 1.0);
	data.dotNL			= clamp(dot(data.normal, data.lightVector), 0.0, 1.0);
	//data.dotNL			= 0.5;
}

ShadingData prepareShadingData(vec3 worldPos, vec3 normal, vec3 tangent, vec2 texCoords, Material mat)
{
	ShadingData data;
	// Default values
	//vec4 normalData		= vec4(0.0, 0.0, 1.0, 0.0);
	vec4 normalData		= vec4(0.0, 0.0, 1.0, 0.0);
	vec4 specularData	= vec4(0.0,0.8,0.0,0.0);
	vec4 diffuseData	= vec4(0.5,0.5,0.5,1.0);
	vec4 ambientData	= vec4(0.0,0.0,0.0,0.0);
	// Read textures
	if(prd.recursion == -1)
	{
		if(mat.tex_idx_diffuse!=MAX_UINT)diffuseData		= texture(sampler2D(tex[mat.tex_idx_diffuse], smp),	texCoords);
		if(mat.tex_idx_normal!=MAX_UINT)normalData			= texture(sampler2D(tex[mat.tex_idx_normal], smp),		texCoords);
		if(mat.tex_idx_specular!=MAX_UINT)specularData		= texture(sampler2D(tex[mat.tex_idx_specular], smp),	texCoords);
		//if(mat.tex_idx_ambient!=MAX_UINT)ambientData		= texture(sampler2D(tex[mat.tex_idx_diffuse], smp),	texCoords);
		data.normal					= calculateNormal(normalData, normal, tangent);
	}
	else
	{
		data.normal = normal;
	}


	if(mat.tex_idx_ambient!=MAX_UINT)
	{
		diffuseData = vec4(0.0);
		ambientData =  vec4(0.1*(int((gl_InstanceCustomIndexEXT))%10), 0.1*((int((gl_InstanceCustomIndexEXT))/10)%10),  0.1*((int((gl_InstanceCustomIndexEXT))/100)%10), 1.0);
	}

	// Write Shading Data
	data.view					= normalize(-gl_ObjectRayDirectionEXT);
	data.metalicity				= specularData.b;
	data.roughness				= clamp(specularData.g, 0.01, 1.0);
	data.diffuse_albedo			= diffuseData.rgb;
	data.fresnel_0				= mix(vec3(0.02f), data.diffuse_albedo, specularData.g);
	//data.roughness				= clamp(specularData.r, 0.01, 1.0);
	data.PI						= 3.141592f;
	data.diffuse_color			= (1.0 - data.metalicity) * data.diffuse_albedo;
	data.specular_color			= vec3(1.0);
	data.ambient_color			= ambientData.rgb;
	
	//prd.hitValue = specularData.rgb;

	data.worldPos				= worldPos;
	return data;
}

void main()
{
	//if(prd.recursion!=0){prd.hitValue = vec3(0.1,0.1,0.1);return;}


	

	//##################### Get Data ############################
	InstanceDescriptor	instance	= inst[gl_InstanceCustomIndexEXT];
	Indices				indices     = Indices(instance.indices);
	Vertices			vertices    = Vertices(instance.vertices);
	Materials			materials   = Materials(instance.materials);

	Material	mat = materials.d[gl_GeometryIndexEXT];
	ivec3		ind = indices.d[gl_PrimitiveID + mat.indexOffset/3];

	//if(gl_GeometryIndexEXT != 40){RETURN_COLOR(vec3(0.0,0.0,0.0));}

	Vertex		v0 = vertices.d[ind.x];
	Vertex		v1 = vertices.d[ind.y];
	Vertex		v2 = vertices.d[ind.z];

	//RETURN_COLOR(v0.pos);
	//RETURN_COLOR(vec3(v0.uvCoord,0.0));
	//RETURN_HASHED(v0.matIdx);
	//if(gl_PrimitiveID<10000){RETURN_COLOR(vec3(1.0,0.0,0.0))}
	//RETURN_COLOR(-v0.pos*0.05);


	//##################### Interpolate ############################
	const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	vec3 modelNormal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
	vec3 normal = (instance.modelMat*vec4(modelNormal, 0.0)).xyz;

	vec3 modelTangent,tangent,modelPos;
	vec2 texCoords;
	//if(prd.recursion == -1)
	//{
		modelTangent = v0.tangent * barycentrics.x + v1.tangent * barycentrics.y + v2.tangent * barycentrics.z;
		tangent = (instance.modelMat*vec4(modelTangent, 0.0)).xyz;
		modelPos = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
		texCoords = v0.uvCoord * barycentrics.x + v1.uvCoord * barycentrics.y + v2.uvCoord * barycentrics.z;
	//}
	vec3 worldPos = (instance.modelMat*vec4(modelPos, 1.0)).xyz;

	//##################### Write Shading Data ############################
	ShadingData data = prepareShadingData(worldPos, normal, tangent, texCoords, mat);

	//return;
	
	//##################### Shading ############################
	prd.recursion++;
	//const uint[3] diffuse_ray_counts		= {2,1,0};
	//const uint[3] specular_ray_counts		= {1,1,0};
	//const uint[3] shadow_ray_counts			= {4,2,1};

	const uint[3] diffuse_ray_counts		= {2,0,0};
	const uint[3] specular_ray_counts		= {2,0,0};
	const uint[3] shadow_ray_counts			= {1,1,1};

	uint diffuse_rays = diffuse_ray_counts[prd.recursion];
	uint specular_rays = specular_ray_counts[prd.recursion];
	uint shadow_rays = shadow_ray_counts[prd.recursion];

	vec3 colorAccumulated = vec3(0,0,0);
	vec3 origin = data.worldPos;// + data.normal*0.001;
	float tMin = 0.02;
	float tMax = 100.0;
	uint rayFlags  = gl_RayFlagsNoneEXT;
	uint rngState = uint(abs(worldPos.x)*10000000+abs(worldPos.y)*1000000+abs(worldPos.z)*1000);

	float coef = 1.0 / float(diffuse_rays + specular_rays + shadow_rays);
	
	colorAccumulated += data.ambient_color*3.0;

	for(uint i = 0; i < diffuse_rays; i++)
	{
		prd.hitValue	= vec3(0,0,0);
		vec3 direction	= randomDirection(rngState, data.normal, 0.8);
        traceRayEXT(topLevelAS, // acceleration structure
              rayFlags,         // rayFlags
              0xFF,             // cullMask
              0,                // sbtRecordOffset
              0,                // sbtRecordStride
              0,                // missIndex
              origin.xyz,       // ray origin
              tMin,             // ray min range
              direction.xyz,    // ray direction
              tMax,             // ray max range
              0                 // payload (location = 0)
        );
		calculateShadingVectorsForDirectionalLight(data, direction);
		vec3 brdf = evaluateDiffuse(data);
		vec3 color = brdf*data.dotNL * prd.hitValue;
        colorAccumulated += 10.0*color/float(diffuse_rays);
	}

	for(uint i = 0; i < specular_rays; i++)
	{
		prd.hitValue	= vec3(0,0,0);
		vec3 direction	= randomDirection(rngState, reflect(-data.view, data.normal), 0.1);
		calculateShadingVectorsForDirectionalLight(data, direction);
		vec3 brdf = evaluateSpecular(data);
		if(brdf.r+brdf.g+brdf.b>0.5 || data.metalicity > 0.2)
		{
			traceRayEXT(topLevelAS, // acceleration structure
			        rayFlags,         // rayFlags
			        0xFF,             // cullMask
			        0,                // sbtRecordOffset
			        0,                // sbtRecordStride
			        0,                // missIndex
			        origin.xyz,       // ray origin
			        tMin,             // ray min range
			        direction.xyz,    // ray direction
			        tMax,             // ray max range
			        0                 // payload (location = 0)
			);
			vec3 color = brdf*data.dotNL * prd.hitValue;
			colorAccumulated += 1.0*color/float(specular_rays);
		}
	}
	//coef=0.05;
	for(uint i = 0; i < shadow_rays; i++)
	{
		prd.hitValue	= vec3(0,0,0);
		//vec3 ligthDirection = pfc.directionalLight.xyz;
		vec3 ligthDirection = normalize(vec3(0.1,-0.5,-0.5));
		vec3 direction	= randomDirection(rngState, ligthDirection, 0.1);
		//vec3 direction	= pfc.directionalLight.xyz;
		//direction = vec3(0.0,-1.0,0.0);
		isShadowed	= true;
        traceRayEXT(topLevelAS, // acceleration structure
              rayFlags | gl_RayFlagsSkipClosestHitShaderEXT,         // rayFlags
              0xFF,             // cullMask
              0,                // sbtRecordOffset
              0,                // sbtRecordStride
              1,                // missIndex
              origin.xyz,       // ray origin
              tMin,             // ray min range
              direction.xyz,    // ray direction
              tMax,             // ray max range
              1                 // payload (location = 1)
        );
		if(!isShadowed)
		{
			calculateShadingVectorsForDirectionalLight(data, direction);
			vec3 brdf = evaluateBRDF(data);
			float lightIntensity = pfc.directionalLight.w * 10.0;
			//vec3 ligthColor = vec3(1.0,0.8,0.6);
			vec3 ligthColor = vec3(1.0,0.5,0.1)*0.4+vec3(0.2,0.3,0.9);
			vec3 color = brdf*data.dotNL * ligthColor * lightIntensity;
			colorAccumulated += color/float(shadow_rays);
		}
	}
	//RETURN_COLOR(colorAccumulated);

	prd.hitValue = colorAccumulated;
	prd.recursion--;
}