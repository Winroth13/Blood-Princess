#include "../shared_resources.hlsli"

RWTexture2D<float4> backBufferUAV;

Texture2D<float4> albedoMetallicTexture : register(t0);
Texture2D<float4> normalRoughnessTexture : register(t1);

StructuredBuffer<DirectionalLight> directionalLights : register(t2);
StructuredBuffer<SpotLight> spotLights : register(t3);
StructuredBuffer<PointLight> pointLights : register(t4);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint width, height;
    backBufferUAV.GetDimensions(width, height);

    if (id.x >= width || id.y >= height)
        return;
        
    uint2 screenPos = id.xy;
    
    float3 color = directionalLights[0].colour; //albedoMetallicTexture[screenPos].rgb;
    
    backBufferUAV[screenPos] = float4(color, 1.0f);
}