#ifndef _FRAME_SHADER_H_
#define _FRAME_SHADER_H_

// Rename this file as appropriate.

// Ideally mirrors struct FrameContextUBO from C++ side...
struct FrameContext
{
	mat4 projection;
	mat4 view;
	mat4 viewProjection;
	vec4 cameraPosition;
	vec4 renderTimer;
};

#endif//INCLUDE_GUARD
