#version 460
#extension GL_EXT_nonuniform_qualifier : require
layout (location = 0)in vec2 Frag_UV;
layout (location = 1)in vec4 Frag_Color;

layout (set=0, binding = 0) uniform texture2D Texture;
layout (set=0, binding = 1) uniform sampler basicSampler;
layout (location = 0) out vec4 out_Color;

void main() 
{
	out_Color = Frag_Color * texture(sampler2D(Texture,basicSampler), Frag_UV.st);
}