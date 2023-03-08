float pi = 3.1415;
float EPSILON = 0.0001;
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

    float internalCacl = NdotH_sqr * (aGaG - 1.0);

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
    
    float D = D_GGX(N,H,alpha);
    vec3 F = Ks + (1.0-Ks) * pow(1-LH,5);
    float G = G_GGX(L,H,alpha) * G_GGX(V,H,alpha);
    
    if (isnan(D))
    {
        return vec3(0.0);
    }
    vec3 result = Kd / pi + D * F / 4.0 * G;   

    return result;
}
