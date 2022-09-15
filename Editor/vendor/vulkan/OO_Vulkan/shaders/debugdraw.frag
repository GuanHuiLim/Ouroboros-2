layout(location = 0) in vec3 inCol;
layout(location = 0) out vec4 outColour;

void main()
{
	outColour = vec4(inCol,1.0);
}