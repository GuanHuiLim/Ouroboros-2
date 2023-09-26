#include "shader_utility.shader"
#include "shared_structs.h"

struct SurfaceProperties
{
    vec3 albedo;
    float roughness;
    float metalness;
    vec3 lightCol;
    float lightRadius;
    vec3 N;
    vec3 V;
    vec3 L;
    vec3 H;
    float dist;
};

struct PBRInfo
{
    float NdotL; // cos angle between normal and light direction
    float NdotV; // cos angle between normal and view direction
    float NdotH; // cos angle between normal and half vector
    float LdotH; // cos angle between light direction and half vector
    float VdotH; // cos angle between view direction and half vector
    float perceptualRoughness; // roughness value, as authored by the model creator (input to shader)
    float metalness; // metallic value at the surface
    vec3 reflectance0; // full reflectance color (normal incidence angle)
    vec3 reflectance90; // reflectance color at grazing angle
    float alphaRoughness; // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 diffuseColor; // color contribution from diffuse lighting
    vec3 specularColor; // color contribution from specular lighting
    float ambientScale;
};

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
    return (alpha+2.0)/ (2*PI) * pow(dot(N,H), alpha);
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
    
    return Kd/PI + D*F/4.0 * G;
}

float D_Beckham(vec3 N, vec3 H, float alpha)
{
    float alphaB = sqrt(2.0/ (alpha+2));
    float aBaB = alphaB*alphaB;
    float vTan = approx_tan(H,N);
    float NdotH = dot(N,H);

    return 1.0/ (PI*aBaB * NdotH*NdotH) * exp(-vTan*vTan/aBaB);
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
    
    return Kd/PI + D*F/4.0 * G;
}

float D_GGX(vec3 N, vec3 H , float alpha)
{
    //float alphaG = sqrt(2.0/ (alpha+2));
    float alphaG = alpha;
    float a2 = pow(alphaG,2);
    //float invAG = pow(alphaG-1.0,2);
    float NdotH = dot(N,H);
    float NdotH_sqr = NdotH * NdotH;

    float internalCacl = NdotH_sqr * (a2 - 1.0) + 1.0;

    //if (internalCacl == -1.0)
    //{
    //    return 1.0;
    //}

    float denom = (PI * pow( internalCacl + 1.0 , 2));
    float result = a2 / (denom+0.0001);
    
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
    
    vec3 result = Kd / PI + D * F / 4.0 * G;   
   
    return result;
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

float Sascha_D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2) / (PI * denom * denom);
}

float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

vec3 F_Schlick(float cosTheta, float metallic, vec3 albedo)
{
    vec3 F0 = mix(albedo, vec3(0.04), metallic); // * material.specular
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    return F;
}



vec3 SaschaBRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness, vec3 lightColor, vec3 albedo)
{
    // Precalculate vectors and dot products	
    vec3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);

	// Light color fixed

    vec3 color = vec3(0.0);

    //if (dotNL > 0.0)
    {
		// D = Normal distribution (Distribution of the microfacets)
        float D = Sascha_D_GGX(dotNH, roughness);
		// G = Geometric shadowing term (Microfacets shadowing)
        float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
        vec3 F = F_Schlick(dotNV, metallic, albedo);

        vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);

        color += spec * dotNL * lightColor;
    }

    return color;
}

// Shlick's approximation of Fresnel
// https://en.wikipedia.org/wiki/Schlick%27s_approximation
vec3 Fresnel_Shlick(in vec3 f0, in vec3 f90, in float x)
{
    return f0 + (f90 - f0) * pow(1.f - x, 5);
}

// Burley B. "Physically Based Shading at Disney"
// SIGGRAPH 2012 Course: Practical Physically Based Shading in Film and Game Production, 2012.
float DiffuseBurley(in float NdotL, in float NdotV, in float LdotH, in float roughness)
{
    float fd90 = 0.5f + 2.f * roughness * LdotH * LdotH;
    return Fresnel_Shlick(vec3(1, 1, 1), vec3(fd90, fd90, fd90), NdotL).x * Fresnel_Shlick(vec3(1, 1, 1), vec3(fd90, fd90, fd90), NdotV).x;
}

// GGX specular D (normal distribution)
// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
float Specular_D_GGX(in float alpha, in float NdotH)
{
    const float alpha2 = alpha * alpha;
    const float lower = (NdotH * NdotH * (alpha2 - 1)) + 1;
    return alpha2 / max(1e-5, PI * lower * lower);
}

// Schlick-Smith specular G (visibility) with Hable's LdotH optimization
// http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
// http://graphicrants.blogspot.se/2013/08/specular-brdf-reference.html
float G_Shlick_Smith_Hable(float alpha, float LdotH)
{
    return 1.0 / (mix(LdotH * LdotH, 1.0, alpha * alpha * 0.25f) + 0.001);
}

// A microfacet based BRDF.
//
// alpha:           This is roughness * roughness as in the "Disney" PBR model by Burley et al.
//
// specularColor:   The F0 reflectance value - 0.04 for non-metals, or RGB for metals. This follows model 
//                  used by Unreal Engine 4.
//
// NdotV, NdotL, LdotH, NdotH: vector relationships between,
//      N - surface normal
//      V - eye normal
//      L - light normal
//      H - half vector between L & V.
vec3 SpecularBRDF(in float alpha, in vec3 specularColor, in float NdotV, in float NdotL, in float LdotH, in float NdotH)
{
    // Specular D (microfacet normal distribution) component
    float specular_D = Specular_D_GGX(alpha, NdotH);

    // Specular Fresnel
    vec3 specular_F = Fresnel_Shlick(specularColor, vec3(1,1,1), LdotH);

    // Specular G (visibility) component
    float specular_G = G_Shlick_Smith_Hable(alpha, LdotH);

    return specular_D * specular_F * specular_G;
}

vec3 EvalLight(in LocalLightInstance lightInfo
                , in vec3 fragPos
                ,in vec3 cameraPos
                , in vec3 normal
                , float roughness
                , in vec3 albedo
                , float metalness)
{
    vec3 N = normal;
    
    // Viewer to fragment
    vec3 V = normalize(cameraPos - fragPos);
    
    // Vector to light
    vec3 L = lightInfo.position.xyz - fragPos;

	// Distance from light to fragment position
    float dist = length(L);
	
	// Light to fragment
    L = normalize(L);
                    
    vec3 H = normalize(L + V);
    float NdotL = max(0.0, dot(N, L));
    
    vec4 lightColInten = lightInfo.color;
    vec3 lCol = lightColInten.rgb * lightColInten.w;
    
    vec3 specular = vec3(0.0);
    metalness = clamp(metalness, 0.04, 0.95f);
    roughness = clamp(roughness, 0.04, 0.95f);
    specular += SaschaBRDF(L, V, N, metalness, roughness, lCol, albedo.rgb);
    
    float r1 = lightInfo.radius.x;
    float atten = UnrealFalloff(dist, lightInfo.radius.x);
    atten = 1.0;
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    result += specular * atten;
   
    if (false)
    {
		//distribute the light across the area
       
    	// Attenuation
        atten = getSquareFalloffAttenuation(L, 1.0 / lightInfo.radius.x);
        
		// Specular part
        float alpha = roughness;
        vec3 Kd = albedo;
        vec3 Ks = vec3(metalness);
        vec3 diff = GGXBRDF(L, V, H, N, alpha, Kd, Ks) * NdotL * atten * lCol;

        float modifySpecular = 1.0;
        //modifySpecular = PC.specularModifier;
        
        vec3 R = -reflect(L, N);
        float RdotV = max(0.0, dot(R, V));
        vec3 spec = lCol * metalness
		* pow(RdotV, max(modifySpecular, 1.0))
		* atten;
        
        result = diff + spec;
    }
    
    return vec3(NdotL);
    return result;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 EvalDirectionalLight(in SurfaceProperties surface
                        , in vec3 irradiance
                        , in vec3 prefilteredColor
                        ,in vec2 lutVal
)
{
    vec3 N = surface.N;    
    // Viewer to fragment
    vec3 V = surface.V;    
    // Vector to light
    vec3 L = surface.L;
    // Half vector
    vec3 H = surface.H;
    
    vec3 lCol = surface.lightCol;    
    float metalness = surface.metalness;
    float roughness = surface.roughness;
    
    
    vec3 baseDiffusePBR = mix(surface.albedo, vec3(0, 0, 0), metalness);
   
    // Specular coefficiant - fixed reflectance value for non-metals
    const float kSpecularCoefficient = 0.04f;
    vec3 baseSpecular = mix(vec3(kSpecularCoefficient), baseDiffusePBR, metalness); //* occlusion;
    
    float NdotV = clamp(dot(N, V), 0.0, 1.0);

    // Burley roughness bias
    float alpha = roughness * roughness;

    // products
    float NdotL = clamp(dot(N, L), 0.0, 1.0);
    float LdotH = clamp(dot(L, H), 0.0, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);

    // Diffuse & specular factors
    float diffuseTerm = DiffuseBurley(NdotL, NdotV, LdotH, roughness);
    vec3 specularTerm = SpecularBRDF(alpha, baseSpecular, NdotV, NdotL, LdotH, NdotH);
    
    
    vec3 finalColor = NdotL * lCol * ((baseDiffusePBR * diffuseTerm) + specularTerm);
    
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), 1.0 - baseDiffusePBR, roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metalness;
    vec3 diffuse = irradiance * surface.albedo;
    vec3 ambient = (kD * diffuse); // * ao;
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), baseDiffusePBR, roughness);
    vec2 envBRDF = lutVal;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    
    ambient = kD * diffuse + specular;
    return specular;
    return finalColor +ambient;
}

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, float roughness, vec3 normal)
{
   // Maps a 2D point to a hemisphere with spread based on roughness
    float alpha = roughness * roughness;
    float phi = 2.0 * PI * Xi.x + random(normal.xz) * 0.1;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha * alpha - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	// Tangent space
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, normal));
    vec3 tangentY = normalize(cross(normal, tangentX));

	// Convert to world Space
    return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
vec3 diffuse(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor / PI;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs)
{
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (PI * f * f);
}

vec3 getIBLContribution(PBRInfo pbrInputs,vec3 irradiance, vec2 brdf, vec3 prefiltedCol)
{
    const float MAX_REFLECTION_LOD = 6.0;
    float lod = (pbrInputs.perceptualRoughness * MAX_REFLECTION_LOD);
	// retrieve a scale and bias to F0. See [1], Figure 3
    vec3 diffuseLight = irradiance.rgb;

    vec3 specularLight = prefiltedCol.rgb;

    vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
    vec3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y);

	// For presentation, this allows us to disable IBL terms
	// For presentation, this allows us to disable IBL terms
    diffuse *= pbrInputs.ambientScale;
    specular *= pbrInputs.ambientScale;

    return diffuse + specular;
}

vec3 SaschaWillemsEvalLight(in SurfaceProperties surface
                        , in vec3 irradiance
                        , in vec3 prefilteredColor
                        , in vec2 lutVal
)
{
    vec3 N = surface.N;
    // Viewer to fragment
    vec3 V = surface.V;
    // Vector to light
    vec3 L = surface.L;
    // Half vector
    vec3 H = surface.H;
    
    vec3 lCol = surface.lightCol;
    float metallic = surface.metalness;
    float roughness = surface.roughness;
    
    
    vec3 f0 = vec3(0.04);
    vec3 diffuseColor = surface.albedo.rgb * (vec3(1.0) - f0);
    diffuseColor *= 1.0 - metallic;
    
    float alphaRoughness = roughness * roughness;
    vec3 specularColor = mix(f0, surface.albedo.rgb, metallic);
    
    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    
    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
	// For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 specularEnvironmentR0 = specularColor.rgb;
    vec3 specularEnvironmentR90 = vec3(1.0) * reflectance90;
    
    float NdotL = clamp(dot(N, L), 0.001, 1.0);
    float NdotV = clamp(abs(dot(N, V)), 0.001, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float LdotH = clamp(dot(L, H), 0.0, 1.0);
    float VdotH = clamp(dot(V, H), 0.0, 1.0);
    
    PBRInfo pbrInputs = PBRInfo(
        NdotL,
		NdotV,
		NdotH,
		LdotH,
		VdotH,
		surface.roughness,
		surface.metalness,
		specularEnvironmentR0,
		specularEnvironmentR90,
		alphaRoughness,
		diffuseColor,
		specularColor,
        1.0f
);
    
    vec3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);
    
    const vec3 u_LightColor = surface.lightCol;
    
    // Calculation of analytical lighting contribution
    vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
    vec3 color = NdotL * u_LightColor * (diffuseContrib + specContrib);

	// Calculate lighting contribution from image based lighting source (IBL)
    // color += getIBLContribution(pbrInputs, irradiance, lutVal, prefilteredColor);
    
    return color;
}

vec3 SaschaWillemsDirectionalLight(in SurfaceProperties surface
                        , in vec3 irradiance
                        , in vec3 prefilteredColor
                        , in vec2 lutVal
)
{
    vec3 N = surface.N;
    // Viewer to fragment
    vec3 V = surface.V;
    // Vector to light
    vec3 L = surface.L;
    // Half vector
    vec3 H = surface.H;
    
    vec3 lCol = surface.lightCol;
    float metallic = surface.metalness;
    float roughness = surface.roughness;
    
    
    vec3 f0 = vec3(0.04);
    vec3 diffuseColor = surface.albedo.rgb * (vec3(1.0) - f0);
    diffuseColor *= 1.0 - metallic;
    
    float alphaRoughness = roughness * roughness;    
    vec3 specularColor = mix(f0, surface.albedo.rgb, metallic);
    
    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    
    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
	// For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 specularEnvironmentR0 = specularColor.rgb;
    vec3 specularEnvironmentR90 = vec3(1.0) * reflectance90;
    
    float NdotL = clamp(dot(N, L), 0.001, 1.0);
    float NdotV = clamp(abs(dot(N, V)), 0.001, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float LdotH = clamp(dot(L, H), 0.0, 1.0);
    float VdotH = clamp(dot(V, H), 0.0, 1.0);
    
    PBRInfo pbrInputs = PBRInfo(
        NdotL,
		NdotV,
		NdotH,
		LdotH,
		VdotH,
		surface.roughness,
		surface.metalness,
		specularEnvironmentR0,
		specularEnvironmentR90,
		alphaRoughness,
		diffuseColor,
		specularColor,
        1.0f
);
    
    vec3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);
    
    const vec3 u_LightColor = surface.lightCol;
    
    // Calculation of analytical lighting contribution
    vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
    vec3 color = NdotL * u_LightColor * (diffuseContrib + specContrib);

	// Calculate lighting contribution from image based lighting source (IBL)
    color += getIBLContribution(pbrInputs, irradiance, lutVal, prefilteredColor);
    
    return color;

}