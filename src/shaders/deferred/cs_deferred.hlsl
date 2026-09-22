#include "../shared_resources.hlsli"
#include "pbr.hlsli"

RWTexture2D<float4> backBufferUAV;

Texture2D<float4> albedoMetallicTexture : register(t0);
Texture2D<float4> normalRoughnessTexture : register(t1);
Texture2D<float4> depthStencilTexture : register(t2);

StructuredBuffer<DirectionalLight> directionalLights : register(t3);
StructuredBuffer<SpotLight> spotLights : register(t4);
StructuredBuffer<PointLight> pointLights : register(t5);

float3 ReconstructWorldPosition(float2 uv, float depth)
{
    float2 ndc = float2(2.0f * uv.x - 1.0f, -2.0f * uv.y + 1.0f);
    float4 positionClip = float4(ndc, depth, 1.0f);
    float4 positionWorld = mul(positionClip, invViewProjectionMatrix);
    return positionWorld.xyz / positionWorld.w;
}

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    backBufferUAV.GetDimensions(width, height);

    if (id.x >= width || id.y >= height)
        return;
            
    int2 pixel = int2(id.xy);
    float2 positionScreen = float2(pixel) + 0.5f;
    float2 uv = positionScreen / float2(width, height);
    
    float depth = depthStencilTexture[pixel].r;
    
    float4 albedoMetallicSample = albedoMetallicTexture[pixel];
    float3 albedoColour = albedoMetallicSample.rgb;
    float metallic = albedoMetallicSample.a;

    float4 normalRoughnessSample = normalRoughnessTexture[pixel];
    float3 normalV = normalize(normalRoughnessSample.rgb * 2.0f - 1.0f);
    float roughness = normalRoughnessSample.a;
    
    float3 positionWorld = ReconstructWorldPosition(uv, depth);
    float3 viewV = normalize(cameraPosition - positionWorld);
    
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedoColour, metallic);
    
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < pointLightCount; ++i)
    {
        PointLight light = pointLights[i];
        
        // calculate per light radiance
        float3 L = normalize(light.position - positionWorld);
        float3 H = normalize(viewV + L);
        float distance = length(light.position - positionWorld);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = light.colour * attenuation;
        
        // cook-torrance brdf
        float NDF = DistributionGGX(normalV, H, roughness);
        float G = GeometrySmith(normalV, viewV, L, roughness);
        float3 F = FresnelSchlick(max(dot(H, viewV), 0.0), F0);
        
        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;
        
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normalV, viewV), 0.0) * max(dot(normalV, L), 0.0) + 0.0001;
        float3 specular = numerator / denominator;
        
        // add to outgoing radiance Lo
        float NdotL = max(dot(normalV, L), 0.0);
        Lo += (kD * albedoColour / PI + specular) * radiance * NdotL;
    }
    
    float3 ambient = ambientColour * albedoColour;
    float3 color = ambient + Lo;
   
    // Remove this later (Gamma Correct and Tonemap)
    // color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, 1.0 / 2.2);

    backBufferUAV[pixel] = float4(color, 1.0f);
}