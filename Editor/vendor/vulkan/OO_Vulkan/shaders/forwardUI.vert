#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : require

#include "shared_structs.h"
#include "instancing.shader"

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inUV; // tex coord, texid , entity
layout(location = 2) in vec4 inCol;

//layout(location = 2) in mat4 inXform;
//
//
//
//layout(location = 6) in uvec4 inInstanceData;


// Note: Sending too much stuff from VS to FS can result in bottleneck...
layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec4 outColor;
layout(location = 3) out flat uvec4 outInstanceData;
layout(location = 4) out vec4 outPrevPosition;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};


layout(std430, set = 0, binding = 3) readonly buffer GPUScene
{
	GPUTransform GPUScene_SSBO[];
};

void main()
{
	outUV = inUV.xy;

	uint isSDFFont = uint(inPosition.w<0.0);

	outInstanceData = uvec4(inUV.zw,isSDFFont,0);
	
	outColor = inCol;
	
    outPosition = uboFrameContext.viewProjJittered * vec4(inPosition.xyz, 1.0);
    gl_Position = outPosition;
    outPrevPosition = uboFrameContext.prevViewProjJittered * vec4(inPosition.xyz, 1.0);
	
}
