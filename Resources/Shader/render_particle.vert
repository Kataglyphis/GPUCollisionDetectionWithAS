#version 460
#extension GL_GOOGLE_include_directive : enable

#include "render_common.glsl"

out gl_PerVertex {
	vec4 gl_Position;
	//float gl_PointSize;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 velocity;
layout(location = 3) in vec3 acceleration;

//layout(location = 0) out vec2 uvCoord;
//layout(location = 1) out vec3 outColor;

layout(binding = 0) uniform PFC
{
	PerFrameConstants d;
} pfc;



void main()
{
	//uvCoord = vec2(0.0);
	//outColor = vec3(0.0);
	//gl_PointSize = 1.0;
	gl_Position =  pfc.d.view * pfc.d.particleModel * vec4(position, 1.0); // For now *0.1 + vec3(0.1,-1.0,0.0)  
	//gl_Position = vec4(0.0,0.0,-0.5,1.0);
	//gl_Position = vec4(1.0,1.0,1.0,1.0);
	//gl_Position = vec4(0.0,0.0,0.0,1.0);
}