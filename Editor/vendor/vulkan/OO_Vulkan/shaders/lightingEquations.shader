#include "shader_utility.shader"

float approx_tan(vec3 V, vec3 N)
{
    float VN = clamp(dot(V,N),EPSILON,1.0);
    return sqrt( 1.0-pow(VN,2) )/VN;
}

float G_SOSS_Approximaton(vec3 L, vec3 H)
{
    return 1.0/ pow(dot(L,H),2);
}

float G_Phong_Beckman_impl(float a)
{
    if(a < 1.6) return 1.0;

    float aa= pow(a,2);
    return (3.535*a + 2.181*aa)/(1.0+2.276*a+ 2.577*aa);
}


float D_Phong(vec3 N, vec3 H, float alpha)
{
    return (alpha+2.0)/ (2*pi) * pow(dot(N,H), alpha);
}

float G_Phong(vec3 V, vec3 H, float a)
{
    float vTan = approx_tan(V,H);
    if(vTan == 0.0) return 1.0;
    float alpha = sqrt(a/2+1.0) / vTan;
    return G_Phong_Beckman_impl(alpha);
}

vec3 PhongBRDF(vec3 L ,vec3 V , vec3 H , vec3 N , float alpha , vec3 Kd , vec3 Ks)
{
    float LH = dot(L,H);
    
    float D = D_Phong(N,H,alpha); // Ditribution
    vec3 F = Ks + (1.0-Ks) * pow(1-LH,5); // Fresnel approximation
    float G = G_Phong(V,H,alpha)* G_Phong(L,H,alpha); // Self-occluding self-shadowing
    
    return Kd/pi + D*F/4.0 * G;
}

float D_Beckham(vec3 N, vec3 H, float alpha)
{
    float alphaB = sqrt(2.0/ (alpha+2));
    float aBaB = alphaB*alphaB;
    float vTan = approx_tan(H,N);
    float NdotH = dot(N,H);

    return 1.0/ (pi*aBaB * NdotH*NdotH) * exp(-vTan*vTan/aBaB);
}

float G_Beckman(vec3 V, vec3 H, float a){
    float vTan = approx_tan(V,H);
    if(vTan == 0.0) return 1.0;
    float alpha = 1.0/(a*vTan);
    return G_Phong_Beckman_impl(alpha);
}

vec3 BeckhamBRDF(vec3 L ,vec3 V , vec3 H , vec3 N , float alpha , vec3 Kd , vec3 Ks)
{
    float LH = dot(L,H);
    
    float D = D_Beckham(N,H,alpha);
    vec3 F = Ks + (1.0-Ks) * pow(1-LH,5);
    float G = G_Beckman(L,H,alpha) * G_Beckman(V,H,alpha);
    
    return Kd/pi + D*F/4.0 * G;
}

float D_GGX(vec3 N, vec3 H , float alpha)
{
    //float alphaG = sqrt(2.0/ (alpha+2));
    float alphaG = alpha;
    float aGaG = pow(alphaG,2);
    //float invAG = pow(alphaG-1.0,2);
    float NdotH = dot(N,H);
    float NdotH_sqr = NdotH * NdotH;

    float internalCacl = NdotH_sqr * (aGaG - 1.0) + 1.0;

    //if (internalCacl == -1.0)
    //{
    //    return 1.0;
    //}

    float denom = (pi * pow( internalCacl + 1.0 , 2));
    float result = aGaG / (denom+0.0001);
    
    return result;
}

float G_GGX(vec3 V, vec3 M , float a)
{
    //float alphaG = sqrt(2.0/ (a+2));
    float alphaG = a;

    float vTan = approx_tan(V,M);

    return 2.0/ (1.0+ sqrt(1+alphaG*alphaG*vTan*vTan));
}

vec3 GGXBRDF(vec3 L ,vec3 V , vec3 H , vec3 N , float alpha , vec3 Kd , vec3 Ks)
{
    float LH = dot(L,H);
  
    alpha = clamp(alpha, 0.001, 0.999);
    LH = clamp(LH, 0.001, 0.999);
    
    float D = D_GGX(N,H,alpha);
    
    vec3 comp1 = Ks + (vec3(1.0) - Ks);
    float vall = 1.0 - LH;
    float comp2 = pow(1.0 - LH, 5.0);
    vec3 F = Ks + (1.0 - Ks) * pow(1 - LH, 5);
    float G = G_GGX(L,H,alpha) * G_GGX(V,H,alpha);
    
    vec3 result = Kd / pi + D * F / 4.0 * G;   
   
    return result;
}



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

	// perspective divide
    vec4 projCoords = fragPosLightSpace / fragPosLightSpace.w;
	//normalization [0,1] tex coords only.. FOR VULKAN DONT DO Z
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    vec2 uvs = vec2(projCoords.x, projCoords.y);
    uvs = GetShadowMapRegion(gridID, uvs, PC.shadowMapGridDim);
	
	// Flip y during sample
    uvs = vec2(uvs.x, 1.0 - uvs.y);
	
	// Bounds check for the actual shadow map
    float sampledDepth = 0.0;
    float lowerBoundsLimit = 0.000001;
    float boundsLimit = 0.999999;
    if (projCoords.x > boundsLimit || projCoords.x < lowerBoundsLimit
		|| projCoords.y > boundsLimit || projCoords.y < lowerBoundsLimit
		)
    {
        return 1.0;
    }
    else
    {
        sampledDepth = texture(sampler2D(samplerShadows,basicSampler), uvs).r;
    }
    float currDepth = projCoords.z;

    float maxbias = PC.maxBias;
    float mulBias = PC.mulBias;
    float bias = max(mulBias * (1.0 - NdotL), maxbias);
    float shadow = 1.0;
    if (currDepth < sampledDepth - bias)
    {
        if (currDepth > 0 && currDepth < 1.0)
        {
            shadow = 0.20;
        }
    }

    return shadow;
}

float AttenuationFactor(float radius, float dist)
{
    float distsqr = dist * dist;
    float rsqr = radius * radius;
    float drsqr = distsqr + rsqr;
    return 2.0 / (drsqr + dist * sqrt(drsqr));
}

float getSquareFalloffAttenuation(vec3 posToLight, float lightInvRadius)
{
    float distanceSquare = dot(posToLight, posToLight);
    float factor = distanceSquare * lightInvRadius * lightInvRadius;
    float smoothFactor = max(1.0 - factor * factor, 0.0);
    return (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);
}

float UnrealFalloff(float dist, float radius)
{
    float num = clamp(1.0 - pow(dist / radius, 4.0), 0.0, 1.0);
    num = num * num;
    float denom = dist * dist + 1;
    return num / denom;
}

vec3 EvalLight(int lightIndex, in vec3 fragPos, in vec3 normal, float roughness, in vec3 albedo, float specular)
{
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    vec3 N = normalize(normal);
    float alpha = roughness;
    vec3 Kd = albedo;
    vec3 Ks = vec3(specular);
	//Ks = vec3(0);
    
	// Vector to light
    vec3 L = Lights_SSBO[lightIndex].position.xyz - fragPos;

	// Distance from light to fragment position
    float dist = length(L);
	
	// Viewer to fragment
    vec3 V = uboFrameContext.cameraPosition.xyz - fragPos;
	
	// Light to fragment
    L = normalize(L);
    V = normalize(V);
    vec3 H = normalize(L + V);
    float NdotL = max(0.0, dot(N, L));

	//if(dist < Lights_SSBO[lightIndex].radius.x)
	{
		//SpotLightInstance light = SpotLightInstance(Omni_LightSSBO[lightIndex]); 
	    
        float r1 = Lights_SSBO[lightIndex].radius.x;
        float r2 = Lights_SSBO[lightIndex].radius.x * 0.9;
        vec4 lightColInten = Lights_SSBO[lightIndex].color;

		//distribute the light across the area
        float LItensity = lightColInten.w / (4 * pi);
        vec3 lCol = lightColInten.rgb * lightColInten.w;

        float radii = pow(1.0 - pow(dist / r1, 4), 2);
        float Evalue = (LItensity / max(dist * dist, 0.01 * 0.01)) * radii;

       
    		// Attenuation
        float atten = AttenuationFactor(r1, dist);
			//if(atten<0.001) discard;
       
        atten = getSquareFalloffAttenuation(L, 1.0 / Lights_SSBO[lightIndex].radius.x);
        atten = UnrealFalloff(dist, Lights_SSBO[lightIndex].radius.x);
        
		// Diffuse part
        vec3 diff = GGXBRDF(L, V, H, N, alpha, Kd, Ks) * NdotL * atten * lCol;

		// Specular part
		// Specular map values are stored in alpha of albedo mrt
        vec3 R = -reflect(L, N);
        float RdotV = max(0.0, dot(R, V));
        vec3 spec = lCol * specular
		* pow(RdotV, max(PC.specularModifier, 1.0))
		* atten;
		//vec3 spec = lCol  * pow(RdotV, 16.0) * atten;
	
		//result = diff;// + spec;	
        result = diff + spec;
    }

	// calculate shadow if this is a shadow light
    float shadow = 1.0;
    if (Lights_SSBO[lightIndex].info.x > 0)
    {
        if (Lights_SSBO[lightIndex].info.x == 1)
        {
            int gridID = Lights_SSBO[lightIndex].info.y;
            for (int i = 0; i < 6; ++i)
            {
                vec4 outFragmentLightPos = Lights_SSBO[lightIndex].projection * Lights_SSBO[lightIndex].view[i] * vec4(fragPos, 1.0);
                shadow *= ShadowCalculation(lightIndex, gridID + i, outFragmentLightPos, NdotL);
            }
        }
        result *= shadow;
    }
    
   

    return result;
//	return fragPos;
}
