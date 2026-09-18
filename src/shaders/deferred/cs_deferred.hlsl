#include "../shared_resources.hlsli"

RWTexture2D<float4> backBufferUAV;

Texture2D<float4> albedoMetallicTexture : register(t0);
Texture2D<float4> normalRoughnessTexture : register(t1);

struct DirectionalLight
{
    float4x4 viewProjMatrix;
    float3 direction;
    float intensity;
    float3 color;
    float pad0;
};

struct PointLight
{
    float3 position;
    float attenuation;
    float3 color;
    float intensity;
};

struct SpotLight
{
    float4x4 viewProjMatrix;
    float3 position;
    float attenuation;
    float3 color;
    float intensity;
    float3 direction;
    float angle;
};

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    backBufferUAV.GetDimensions(width, height);

    if (id.x >= width || id.y >= height)
        return;
        
    uint2 screenPos = id.xy;
    
    float3 color = albedoMetallicTexture[screenPos].rgb;
    
    backBufferUAV[screenPos] = float4(color, 1.0f);
}