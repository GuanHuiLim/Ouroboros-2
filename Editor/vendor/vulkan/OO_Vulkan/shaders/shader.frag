layout(location = 0) in vec3 inCol;
layout(location = 1) in vec2 fragTex;

layout(set = 2, binding= 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColour; //final output colour (Must also have location!)

void main()
{
	//outColour = texture(textureSampler,fragTex)*vec4(fragCol,1.0);
	outColour = vec4(inCol,1.0);
}