#include "shader_utility.shader"
#include "shared_structs.h"

vec2 GetShadowMapRegion(int gridID, in vec2 uv, in vec2 gridSize)
{
	
    vec2 gridIncrement = vec2(1.0) / gridSize; // size for each cell

    vec2 actualUV = gridIncrement * uv; // uv local to this cell

	// avoid the modolus operator not sure how much that matters
    int y = gridID / int(gridSize.x);
    int x = gridID - int(gridSize.x * y);

    vec2 offset = gridIncrement * vec2(x, y); // offset to our cell

    return offset + actualUV; //sampled position
}

float ShadowCalculation(int lightIndex, int gridID, in vec4 fragPosLightSpace, float NdotL)
{
    const float oneTexelUV = 1.0 / (4096.0);
	// perspective divide
    vec4 projCoords = fragPosLightSpace / fragPosLightSpace.w;
	//normalization [0,1] tex coords only.. FOR VULKAN DONT DO Z
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    vec2 uvs = vec2(projCoords.x, projCoords.y);
    uvs = clamp(uvs, oneTexelUV, 1.0 - oneTexelUV); // clamp between the grids
    uvs = GetShadowMapRegion(gridID, uvs, PC.shadowMapGridDim);
    
   
	// Flip y during sample
    uvs = vec2(uvs.x, 1.0 - uvs.y);
	
    float currDepth = projCoords.z;

    float maxbias = PC.maxBias;
    float mulBias = PC.mulBias;
    float bias = max(mulBias * (1.0 - NdotL), maxbias);
    float shadow = 1.0;
    
	// Bounds check for the actual shadow map
    float sampledDepth = 0.0;
    float shadowFactor = 0.0;
    int count = 0;
    int range = 1;
    
    float lowerBoundsLimit = 0.0 + EPSILON;
    float boundsLimit = 1.0 - EPSILON;    
    
    for (int x = -range; x <= range; x++)
    {
        for (int y = -range; y <= range; y++)
        {
            if (projCoords.x > boundsLimit || projCoords.x < lowerBoundsLimit
		    || projCoords.y > boundsLimit || projCoords.y < lowerBoundsLimit
               || currDepth > boundsLimit
		    )
            {
                shadowFactor += 1.0;
            }
            else
            {        
                shadowFactor += texture(sampler2DShadow(textureShadows, shadowSampler)
                                            , vec3(uvs + vec2(x,y)*oneTexelUV, currDepth + bias)).r;
            }
           count++;
        }
	
    }
    shadow = shadowFactor / count;   
    
    // old code for documentation purpose
    //if (projCoords.x > boundsLimit || projCoords.x < lowerBoundsLimit
	//	    || projCoords.y > boundsLimit || projCoords.y < lowerBoundsLimit
	//	    )
    //{
    //    shadow += 1.0;
    //}
    //else
    //{
    //    shadow = texture(sampler2DShadow(textureShadows, shadowSampler), vec3(uvs, currDepth + bias)).r;        
    //}
            
     //sampledDepth = texture(sampler2D(textureShadows, basicSampler), uvs).r;
    //if (currDepth < sampledDepth - bias)
    //{
    //    if (currDepth > 0 && currDepth < 1.0)
    //    {
    //        shadow = 0.20;
    //    }
    //}

    return shadow;
}

float EvalShadowMap(in LocalLightInstance lightInfo, int lightIndex, in vec3 normal, in vec3 fragPos)
{
    vec3 N = normal;
    vec3 L = normalize(lightInfo.position.xyz - fragPos);
    
   
    
    float NdotL = max(0.0, dot(N, L));
    float shadow = 1.0;
    switch (lightInfo.info.x)
    {
        // this is point light
        case 1:
        {
            int gridID = lightInfo.info.y;           
            // get a cube coordinate for the layer to use
            ivec3 map = texCoordToCube(-L, vec2(1));                
            vec4 outFragmentLightPos = lightInfo.projection * lightInfo.view[map.z] * vec4(fragPos, 1.0);
            shadow = ShadowCalculation(lightIndex, gridID + map.z, outFragmentLightPos, NdotL);
            
        }
        default: return shadow;
    }       
   
    return shadow;
}
