layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 inCol;
layout(location = 4) in vec2 tex;

#include "frame.shader"
layout(set = 1,binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};

layout(push_constant)uniform PushModel
{
		mat4 model;
}pushModel;

layout(location = 0) out vec3 outCol;

void main()
{
	gl_Position = uboFrameContext.viewProjection * vec4(pos, 1.0);

	outCol = inCol;
}
