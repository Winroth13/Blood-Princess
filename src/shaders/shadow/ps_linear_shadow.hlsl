#include "../shared_resources.hlsli"

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D albedoTexture : register(t0);

float main(PixelShaderInput input) : SV_TARGET
{
    float4 albedoSample = albedoTexture.Sample(samplerLinearWrap, input.uv);
    if (albedoSample.a < 0.5f)
        discard;

    float lightDistance = length(input.worldPosition - cameraPosition);
    
    return lightDistance;
}