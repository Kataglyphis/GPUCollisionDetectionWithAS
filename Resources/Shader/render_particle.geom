#version 460
#extension GL_GOOGLE_include_directive : enable
#include "render_common.glsl"
#define SIZE 0.05

#define LEFT_TOP		vec2(-1.0, 1.0)
#define LEFT_BOTTOM		vec2(-1.0, -1.0)
#define RIGHT_TOP		vec2(1.0, 1.0)
#define RIGHT_BOTTOM	vec2(1.0, -1.0)

#define ADD_VERTEX(I,P) gl_Position = pfc.d.projection * (gl_in[(I)].gl_Position + vec4((P).x*SIZE, (P).y*SIZE, 0.0,0.0)); uvCoord = vec2(0.01, (P).y); EmitVertex();

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
layout(location = 0) out vec2 uvCoord;
layout(location = 1) out vec3 color;

layout(binding = 0) uniform PFC
{
	PerFrameConstants d;
} pfc;


//in gl_PerVertex {
//    vec4 gl_Position;
//} gl_in[];


void main()
{
	//ADD_VERTEX(0,LEFT_TOP)
	//ADD_VERTEX(1,LEFT_BOTTOM)
	//ADD_VERTEX(2,RIGHT_TOP)
	//ADD_VERTEX(3,RIGHT_BOTTOM)
	color = gl_in[(0)].gl_Position.xyz;

	gl_Position = pfc.d.projection * (gl_in[(0)].gl_Position + vec4((-1.0)*SIZE, (1.0)*SIZE, 0.0,0.0));
	//uvCoord = vec2(-1.0, 1.0);
	uvCoord = vec2(0.0, 0.0);
	EmitVertex();
	gl_Position = pfc.d.projection * (gl_in[(0)].gl_Position + vec4((-1.0)*SIZE, (-1.0)*SIZE, 0.0,0.0));
	//uvCoord = vec2(-1.0, -1.0);
	uvCoord = vec2(0.0, 1.0);
	EmitVertex();
	gl_Position = pfc.d.projection * (gl_in[(0)].gl_Position + vec4((1.0)*SIZE, (1.0)*SIZE, 0.0,0.0));
	//uvCoord = vec2(1.0, 1.0);
	uvCoord = vec2(1.0, 0.0);
	EmitVertex();
	gl_Position = pfc.d.projection * (gl_in[(0)].gl_Position + vec4((1.0)*SIZE, (-1.0)*SIZE, 0.0,0.0));
	//uvCoord = vec2(1.0, -1.0);
	uvCoord = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}