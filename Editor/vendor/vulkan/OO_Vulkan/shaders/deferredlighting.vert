layout (location = 0) out vec2 outUV;

#include "shared_structs.h"
layout( push_constant ) uniform pc
{
	LightPC lightPC;
};




// this shader creates a fullscreen quad without any vertices
void main() 
{
	vec2 uv = vec2((gl_VertexIndex << 1) & 2 , gl_VertexIndex & 2);
	gl_Position = vec4(uv * 2.0f - 1.0f, 0.0f, 1.0f);
	outUV = vec2((gl_VertexIndex << 1) & 2 , ((~gl_VertexIndex) & 2)-1);
}
