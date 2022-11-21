#extension GL_EXT_nonuniform_qualifier : require


#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};


void main()
{
	// implictly writes to depth buffer
	// gl_FragDepth = gl_FragCoord.z;
}
