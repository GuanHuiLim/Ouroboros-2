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
	outUV = inUV.xy;

	uint isSDFFont = uint(inPosition.w<0.0);

	outInstanceData = uvec4(inUV.zw,isSDFFont,0);
	
	outColor = inCol;

	//if((inInstanceData.y & 0x0f)>0) // billboard
	//{
	//	vec3 fragOffset = inPosition.xyz;
	//	vec3 CameraRight_worldspace = vec3(uboFrameContext.view[0][0], uboFrameContext.view[1][0], uboFrameContext.view[2][0]);
	//	vec3 CameraUp_worldspace = vec3(uboFrameContext.view[0][1], uboFrameContext.view[1][1], uboFrameContext.view[2][1]);
	//	vec3 CameraForward_worldspace = vec3(uboFrameContext.view[0][2], uboFrameContext.view[1][2], uboFrameContext.view[2][2]);
	//	CameraUp_worldspace = -CameraUp_worldspace; // flip y for rendering
	//
	//	vec3 vertexPosition_worldspace =
	//	vec3(inXform[3][0],inXform[3][1],inXform[3][2])
	//	+ CameraRight_worldspace * inXform[0][0] * fragOffset.x
	//	+ CameraUp_worldspace * inXform[1][1] * fragOffset.y;
	//	+ CameraForward_worldspace * inXform[3][3] * fragOffset.z;
	//	
	//	outPosition = vec4(vertexPosition_worldspace,1.0);
	//	outLightData.btn = mat3(CameraRight_worldspace,CameraUp_worldspace,cross(CameraUp_worldspace,CameraRight_worldspace));
	//}
	//else
	//{
	//	outPosition = inXform*vec4(inPosition,1.0);
	//}
	
	gl_Position = uboFrameContext.viewProjection * vec4(inPosition.xyz,1.0);
	//float idx = float(gl_VertexIndex); // this is the vertex id
	//float ins = float(gl_InstanceIndex); // this is the draw call number
	
}
