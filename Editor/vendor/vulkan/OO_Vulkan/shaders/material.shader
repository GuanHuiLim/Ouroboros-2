#ifndef _MATERIAL_SHADER_H_
#define _MATERIAL_SHADER_H_

#include "shader_common.shader"

float2 random2(float2 p)
{
    return fract(sin(float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)))) * 43758.5453);
}

float3 voronoi(float time, in float2 x)
{
    float2 n = floor(x);
    float2 f = fract(x);

    // first pass: regular voronoi
    float2 mg, mr;
    float md = 8.0;
    for (int j = -1; j <= 1; j++)
    {
        for (int i = -1; i <= 1; i++)
        {
            float2 g = float2(float(i), float(j));
            float2 o = random2(n + g);
            o = 0.5 + 0.5 * sin(time + 6.2831 * o);

            float2 r = g + o - f;
            float d = dot(r, r);

            if (d < md)
            {
                md = d;
                mr = r;
                mg = g;
            }
        }
    }

    // second pass: distance to borders
    md = 8.0;
    for (int j = -2; j <= 2; j++)
    {
        for (int i = -2; i <= 2; i++)
        {
            float2 g = mg + float2(float(i), float(j));
            float2 o = random2(n + g);
            o = 0.5 + 0.5 * sin(time + 6.2831 * o);

            float2 r = g + o - f;

            if (dot(mr - r, mr - r) > 0.00001)
            {
                md = min(md, dot(0.5 * (mr + r), normalize(r - mr)));
            }
        }
    }
    return float3(md, mr);
}

float4 main_voronoi(in float time, in float2 uv)
{
    float3 color = float3(0.0);
    float3 c = voronoi(time, uv * 3.0);
    // isolines
    color = c.x * (0.5 + 0.5 * sin(64.0 * c.x)) * float3(1.0);
    // borders
    color = lerp(float3(1.0), color, smoothstep(0.01, 0.02, c.x));
    // feature points
    //float dd = length(c.yz);
    //color += float3(1.) * (1.0 - smoothstep(0.0, 0.04, dd));

    return vec4(color, 1.0);
}

#endif//INCLUDE_GUARD
