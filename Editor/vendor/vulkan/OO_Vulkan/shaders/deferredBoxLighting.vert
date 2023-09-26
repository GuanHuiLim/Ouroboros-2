#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 1) out flat int outLightInstance;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inUV;

#include "shared_structs.h"
layout( push_constant ) uniform pc
{
	LightPC lightPC;
};

#include "lights.shader"

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};

// this shader creates a fullscreen quad without any vertices
void main() 
{

	vec3 L = Lights_SSBO[gl_InstanceIndex].position.xyz;
	float diam =  Lights_SSBO[gl_InstanceIndex].radius.x * 2.0;

	mat4 xform = mat4(
	  diam, 0.0, 0.0, 0.0,
	  0.0, diam, 0.0, 0.0,
	  0.0, 0.0, diam, 0.0,
	  L.x,L.y,L.z, 1.0
	);

	outLightInstance = int(gl_InstanceIndex);

	gl_Position = uboFrameContext.viewProjection* xform * vec4(inPosition,1.0);

}
