#ifndef PBR
#define PBR

#include "../shared_resources.hlsli"

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 ComputePBR(
    float3 normalV, 
    float3 viewV, 
    float3 lightV, 
    float3 halfV,
    
    float3 albedoColour,
    float roughness, 
    float metallic, 
    float3 baseReflectivity, 

    float3 radiance
)
{
    // cook-torrance brdf
    float NDF = DistributionGGX(normalV, halfV, roughness);
    float G = GeometrySmith(normalV, viewV, lightV, roughness);
    float3 F = FresnelSchlick(max(dot(halfV, viewV), 0.0), baseReflectivity);
    
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;
        
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normalV, viewV), 0.0) * max(dot(normalV, lightV), 0.0) + 0.0001;
    float3 specular = numerator / denominator;
    
    float NdotL = max(dot(normalV, lightV), 0.0);
    return (kD * albedoColour / PI + specular) * radiance * NdotL;
}

#endif