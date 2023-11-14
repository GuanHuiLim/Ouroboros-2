#ifndef _FRAME_SHADER_H_
#define _FRAME_SHADER_H_

// Rename this file as appropriate.

// Ideally mirrors struct FrameContextUBO from C++ side...
struct FrameContext
{
	mat4 projection;
	mat4 view;
	mat4 viewProjection;
	mat4 inverseViewProjection;
	mat4 inverseView;
	mat4 inverseProjection;
	vec4 cameraPosition;
    mat4 prevViewProjection;
	vec4 renderTimer;
    mat4 projectionJittered;
    mat4 viewProjJittered;
	mat4 inverseProjectionJittered;
    mat4 prevViewProjJittered;
    vec2 currJitter;
    vec2 prevJitter;

	// These variables area only to speedup development time by passing adjustable values from the C++ side to the shader.
	// Bind this to every single shader possible.
	// Remove this upon shipping the final product.
	vec4 vector4_values0;
	vec4 vector4_values1;
	vec4 vector4_values2;
	vec4 vector4_values3;
	vec4 vector4_values4;
	vec4 vector4_values5;
	vec4 vector4_values6;
	vec4 vector4_values7;
	vec4 vector4_values8;
	vec4 vector4_values9;
};

#endif//INCLUDE_GUARD
