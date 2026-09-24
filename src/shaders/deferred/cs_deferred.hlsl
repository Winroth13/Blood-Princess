#include "../shared_resources.hlsli"
#include "pbr.hlsli"
#include "../shadow/shadows.hlsli"

RWTexture2D<float4> backBufferUAV;

Texture2D<float4> albedoMetallicTexture : register(t0);
Texture2D<float4> normalRoughnessTexture : register(t1);
Texture2D<float4> depthStencilTexture : register(t2);

StructuredBuffer<DirectionalLight> directionalLights : register(t3);
StructuredBuffer<SpotLight> spotLights : register(t4);
StructuredBuffer<PointLight> pointLights : register(t5);

Texture2DArray<float> directionalLightShadowMaps : register(t6);

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
    
    float3 baseReflectivity = float3(0.04f, 0.04f, 0.04f);
    baseReflectivity = lerp(baseReflectivity, albedoColour, metallic);
    
    float3 outRadiance = float3(0.0f, 0.0f, 0.0f);
    
    int i;
    
    /* Point Lights */
    for (i = 0; i < pointLightCount; ++i)
    {
        PointLight light = pointLights[i];
        
        float3 lightV = light.position - positionWorld;
        float distance = length(lightV);
        
        if (distance > light.radius)
            continue;
        
        lightV /= distance;
        float3 halfV = normalize(viewV + lightV);
        float window = saturate(1.0f - pow(distance / light.radius, 4.0f));
        float attenuation = (window * window) / (distance * distance + 1.0f);
        float3 radiance = light.colour * attenuation;
        
        outRadiance += ComputePBR(normalV, viewV, lightV, halfV, albedoColour, roughness, metallic, baseReflectivity, radiance);
    }
    
    /* Spot Lights */
    for (i = 0; i < spotLightCount; ++i)
    {
        SpotLight light = spotLights[i];
        
        float3 lightV = light.position - positionWorld;
        float distance = length(lightV);
        
        if (distance > light.range)
            continue;
        
        lightV /= distance;
        
        float t = dot(spotLights[i].direction, -lightV);
        float coneFactor = smoothstep(light.outerAngleCos, light.innerAngleCos, t);
        if (coneFactor <= 0.0f)
            continue;
        
        float3 halfV = normalize(viewV + lightV);
        float window = saturate(1.0f - pow(distance / light.range, 4.0f));
        float attenuation = (window * window) / (distance * distance + 1.0f);
        float3 radiance = light.colour * attenuation * coneFactor;
        
        outRadiance += ComputePBR(normalV, viewV, lightV, halfV, albedoColour, roughness, metallic, baseReflectivity, radiance);
    }
    
    /* Directional Lights */
    for (i = 0; i < directionalLightCount; ++i)
    {
        float depth = directionalLightShadowMaps.SampleLevel(samplerShadow, float3(0, 0, 0), 0);
        
        DirectionalLight light = directionalLights[i];
        
        float shadowFactor = CalcShadowFactor(
            positionWorld,
            light.viewProjectionMatrix,
            light.shadowArrayIndex,
            directionalLightShadowMaps,
            true
        );
        
        if (shadowFactor <= 1e-5f)
            continue;
        
        float3 lightV = -light.direction;
        float3 halfV = normalize(viewV + lightV);
        float3 radiance = light.colour;
        
        outRadiance += shadowFactor * ComputePBR(normalV, viewV, lightV, halfV, albedoColour, roughness, metallic, baseReflectivity, radiance);
    }
    
    float3 ambient = ambientColour * albedoColour;
    float3 color = ambient + outRadiance;

    backBufferUAV[pixel] = float4(color, 1.0f);
}