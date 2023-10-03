#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : require

#include "shared_structs.h"

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;
layout (location = 2) in vec4 Color;
layout (std140,set = 1, binding = 0) uniform PerFrameData
{
	uniform mat4 MVP;
};

layout (location = 0)out vec2 Frag_UV;
layout (location = 1)out vec4 Frag_Color;

void main()
{
	Frag_UV = UV;
	Frag_Color = Color;
	gl_Position = MVP * vec4(Position.xy,0,1);
}