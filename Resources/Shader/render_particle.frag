#version 460
#extension GL_GOOGLE_include_directive : enable
#include "render_common.glsl"

layout ( location = 0 ) in vec2 uvCoord;
layout ( location = 1 ) in vec3 color;

layout( location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler smp;

layout(binding = 2) uniform texture2D tex[2]; // For now


void main()
{
	outColor = texture(sampler2D(tex[0], smp), uvCoord);
	//outColor.xyz+=color;
	//outColor = vec4(1.0,0.0,0.0,1.0);
	//outColor.a = 0;
	//outColor = vec4(uvCoord.x,uvCoord.y,0.5,1.0);
}