#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : require

#include "shared_structs.h"
#include "instancing.shader"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inUV;

layout(location = 5) in mat4 inXform;
layout(location = 9) in vec4 inCol;
layout(location = 10) in uvec4 inInstanceData;


// Note: Sending too much stuff from VS to FS can result in bottleneck...
layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec4 outColor;
layout(location = 3) out flat uvec4 outInstanceData;
layout(location = 7) out struct
{
	mat3 btn;
}outLightData;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};

//layout (set = 2, binding = 0) uniform sampler2D textureDesArr[];

layout(std430, set = 0, binding = 3) readonly buffer GPUScene
{
	GPUTransform GPUScene_SSBO[];
};

#include "skinning.shader"

layout(std430, set = 0, binding = 5) readonly buffer GPUobject
{
	GPUObjectInformation GPUobjectInfo[];
};

void main()
{
	outUV = inUV;
	outColor = inCol;
	outInstanceData = inInstanceData;
	
	mat3 L2W = mat3(inXform);

	vec3 NN = normalize(inNormal);
	vec3 NT = normalize(inTangent);
	vec3 NB = cross(NN, NT);
	
	vec3 T = normalize(L2W * vec3(NT)).xyz;
	vec3 B = normalize(L2W * vec3(NB)).xyz;
	vec3 N = normalize(L2W * vec3(NN)).xyz;

	outLightData.btn = (mat3(T,B,N));
	
	outPosition = inXform * vec4(inPosition,1.0);
	gl_Position = uboFrameContext.viewProjection * outPosition;
	
}
