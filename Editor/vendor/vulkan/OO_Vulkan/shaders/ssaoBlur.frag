layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
    FrameContext uboFrameContext;
};

#include "shared_structs.h"
layout (set = 0, binding = 0) uniform sampler basicSampler; 
layout (set = 0, binding = 1) uniform texture2D samplerSSAO; 

layout( push_constant ) uniform pc
{
	SSAOPC PC;
};

void main()
{	
    vec2 texelSize = 1.0 / vec2(textureSize(sampler2D(samplerSSAO,basicSampler), 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(sampler2D(samplerSSAO,basicSampler), inUV + offset).r;
        }
    }
    outFragcolor = vec4(result / (4.0 * 4.0));
}
