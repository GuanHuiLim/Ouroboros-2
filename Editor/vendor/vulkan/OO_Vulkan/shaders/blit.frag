layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
    FrameContext uboFrameContext;
};

#include "shared_structs.h"
layout (set = 0, binding = 1) uniform sampler2D samplerSource; 

void main()
{	
    outFragcolor = texture(samplerSource,inUV);
}
