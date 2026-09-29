#include "../shared_resources.hlsli"

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float2 uv : UV;
};

Texture2D albedoTexture : register(t0);

#define FAR_PLANE 64.0f

float main(PixelShaderInput input) : SV_TARGET
{
    float4 albedoSample = albedoTexture.Sample(samplerLinearWrap, input.uv);
    if (albedoSample.a < 0.5f)
        discard;

    float lightDistance = length(input.worldPosition - cameraPosition);

    //lightDistance = lightDistance / FAR_PLANE;
    
    return lightDistance;
}