

// frostbite brdf: https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf

vec3 F_Schlick(vec3 f0, vec3 f90, float u)
{
	float one_minus = clamp(1.0f - u,0.0,1.0);
	//float one_minus = 1.0f - u;
	float one_minus_square = one_minus * one_minus;
	return f0 + (f90 - f0) * (one_minus_square * one_minus * one_minus_square);
}

float V_SmitGGXCorrelated(float dotNL, float dotNV, float alphaG)
{
	float alphaG2 = alphaG * alphaG;

	float lambda_GGXV = max(dotNL * sqrt((- dotNV * alphaG2 + dotNV) * dotNV + alphaG2),0.00001);
	float lambda_GGXL = max(dotNV * sqrt((- dotNL * alphaG2 + dotNL) * dotNL + alphaG2),0.00001);

	return 0.5 / (lambda_GGXV + lambda_GGXL);
}

float D_GGX(float dotNH, float m)
{
	float m2 = m*m;
	float f = (dotNH * m2 - dotNH) * dotNH + 1;
	return m2 / (f * f);
}


vec3 evaluateSpecular(ShadingData data)
{
	vec3 fresnel_90 = vec3(1.0);
	vec3 f = F_Schlick(data.fresnel_0, fresnel_90, data.dotLH);
	float vis = V_SmitGGXCorrelated(data.dotNV, data.dotNL, data.roughness);
	float d = D_GGX(data.dotNH, data.roughness);
	vec3 fr = d * f * vis / data.PI;

	return fr*data.specular_color;
}

float Fr_DisneyDiffuse(float dotNV, float dotNL, float dotLH, float linearRoughness)
{
	float energyBias	= mix(0.0 , 0.5, linearRoughness);
	float energyFactor	= mix(1.0, 1.0 / 1.51, linearRoughness);
	vec3 fresnel_90		= vec3(energyBias + 2.0 * dotLH * dotLH * linearRoughness);
	vec3 fresnel_0		= vec3(1.0);
	float lightScatter	= F_Schlick(fresnel_0, fresnel_90, dotNL).r;
	float viewScatter	= F_Schlick(fresnel_0, fresnel_90, dotNV).r;
	return lightScatter * viewScatter * energyFactor;
}

vec3 evaluateDiffuse(ShadingData data)
{
	float fd = Fr_DisneyDiffuse(data.dotNV, data.dotNL, data.dotLH, sqrt(data.roughness))/data.PI;
	return fd*data.diffuse_color;
}

vec3 evaluateBRDF(ShadingData data)
{
	return evaluateDiffuse(data)+evaluateSpecular(data);
}
